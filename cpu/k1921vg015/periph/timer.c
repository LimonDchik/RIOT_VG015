#include "cpu.h"
#include "periph/timer.h"
#include "K1921VG015.h"
#include "plic.h"

/* -----------------------------------------------------------------------
 * CLINT — машинный таймер ядра RISC-V (K1921VG015, base 0x02000000)
 * clint.h уже определяет CLINT_MTIME/CLINT_MTIMECMP как смещения (0xBFF8,
 * 0x4000). Здесь задаём базовый адрес и удобные accessor-макросы.
 * --------------------------------------------------------------------- */
#define CLINT_BASE      0x02000000U
#define CLINT_MTIME_REG    (*(volatile uint64_t *)(CLINT_BASE + 0xBFF8U))
#define CLINT_MTIMECMP_REG (*(volatile uint64_t *)(CLINT_BASE + 0x4000U))

/* -----------------------------------------------------------------------
 * Контексты прерываний периферийного таймера
 * --------------------------------------------------------------------- */
static timer_isr_ctx_t isr_ctx[TIMER_NUMOF];

/* forward declaration */
void tmrvg015_isr(int num);

/* Вспомогательная функция: возвращает указатель на регистры таймера */
static inline TMR_TypeDef *dev(tim_t tim)
{
    return timer_config[tim].dev;
}

/* =======================================================================
 * timer_init
 * Инициализирует периферийный таймер TMR1 и подавляет MTIP от CLINT.
 * ===================================================================== */
int timer_init(tim_t tim, unsigned long freq, timer_cb_t cb, void *arg)
{
    if (tim >= TIMER_NUMOF) {
        return -1;
    }

    /* --- 1. Заглушить машинный таймер CLINT, чтобы не мешал trap-у --- */
    clear_csr(mie, (1 << 7));               /* запретить MTIE             */
    CLINT_MTIMECMP_REG = CLINT_MTIME_REG + 0xFFFFFFFFULL; /* сдвинуть сравнение вперёд */

    /* --- 2. Сохранить callback --- */
    isr_ctx[tim].cb  = cb;
    isr_ctx[tim].arg = arg;

    /* --- 3. Тактирование и сброс TMR1 --- */
    RCU->CGCFGAPB_bit.TMR1EN  = 1;
    RCU->RSTDISAPB_bit.TMR1EN = 1;

    /* --- 4. Настроить таймер --- */
    dev(tim)->CTRL_bit.MODE = TMR_CTRL_MODE_Stop; /* остановить перед настройкой */
    dev(tim)->COUNT         = 0;
    dev(tim)->CAPCOM[0].VAL = 50000000UL / freq;  /* период = SYSCLK / freq      */
    dev(tim)->CTRL_bit.DIV  = 0;                  /* делитель = 1                */
    dev(tim)->IC            = 0xFFFFFFFFU;         /* сбросить все pending флаги  */
    dev(tim)->IM            = 1;                   /* разрешить прерывание TMR    */

    /* --- 5. Подключить обработчик к PLIC --- */
    plic_set_isr_cb(timer_config[tim].irqn, tmrvg015_isr);
    plic_enable_interrupt(timer_config[tim].irqn);
    plic_set_priority(timer_config[tim].irqn, TMR_INTR_PRIORITY);

    /* --- 6. Запустить таймер в периодическом режиме --- */
    dev(tim)->CTRL_bit.MODE = TMR_CTRL_MODE_Up;

    return 0;
}

/* =======================================================================
 * tmrvg015_isr — обработчик прерывания от TMR1 (вызывается из PLIC)
 * num: номер IRQ (приходит как заглушка из plic_isr_handler)
 * ===================================================================== */
void tmrvg015_isr(int num)
{
    /* Сбросить флаг прерывания в периферии ДО вызова callback,
     * иначе при выходе из ISR прерывание снова сработает немедленно */
    TMR1->IC = 1;

    isr_ctx[TIMER_DEV(0)].cb(isr_ctx[TIMER_DEV(0)].arg, num);
}

/* =======================================================================
 * timer_set — one-shot: сработать через timeout тиков от текущего COUNT
 * ===================================================================== */
int timer_set(tim_t tmr, int channel, unsigned int timeout)
{
    unsigned int now = (unsigned int)dev(tmr)->COUNT;
    return timer_set_absolute(tmr, channel, now + timeout);
}

/* =======================================================================
 * timer_set_absolute — one-shot: сработать когда COUNT == value
 * ===================================================================== */
int timer_set_absolute(tim_t tmr, int channel, unsigned int value)
{
    channel = 1;
    channel = -channel;
    if (tmr >= TIMER_NUMOF) {
        return -1;
    }

    dev(tmr)->CTRL_bit.MODE    = TMR_CTRL_MODE_Stop;
    dev(tmr)->IC               = 0xFFFFFFFFU;  /* сбросить pending флаги */
    dev(tmr)->CAPCOM[0].VAL    = value;         /* CAPCOM[0] — период     */
    dev(tmr)->COUNT            = 0;
    dev(tmr)->IM               = 1;
    dev(tmr)->CTRL_bit.MODE    = TMR_CTRL_MODE_Up;

    return 0;
}

/* =======================================================================
 * timer_set_periodic — периодический режим с заданным периодом value
 * ===================================================================== */
int timer_set_periodic(tim_t tmr, int channel, unsigned int value,
                        uint8_t flags)
{
    (void)channel;
    (void)flags;

    if (tmr >= TIMER_NUMOF) {
        return -1;
    }

    dev(tmr)->CTRL_bit.MODE = TMR_CTRL_MODE_Stop;
    dev(tmr)->IC            = 0xFFFFFFFFU;
    dev(tmr)->CAPCOM[0].VAL = value;
    dev(tmr)->COUNT         = 0;
    dev(tmr)->IM            = 1;
    dev(tmr)->CTRL_bit.MODE = TMR_CTRL_MODE_Up;

    return 0;
}

/* =======================================================================
 * timer_clear — остановить таймер, сбросить счётчик и pending флаг
 * ===================================================================== */
int timer_clear(tim_t tmr, int channel)
{
    (void)channel;

    dev(tmr)->CTRL_bit.MODE = TMR_CTRL_MODE_Stop;
    dev(tmr)->IC            = 0xFFFFFFFFU;  /* сбросить все флаги прерываний */
    dev(tmr)->COUNT         = 0;

    return 0;
}

/* =======================================================================
 * timer_read — прочитать текущее значение счётчика
 * Примечание из datasheet: читать COUNT только при остановленном таймере.
 * Для быстрого чтения «на ходу» допустимо, если точность не критична.
 * ===================================================================== */
unsigned int timer_read(tim_t tmr)
{
    return (unsigned int)dev(tmr)->COUNT;
}

/* =======================================================================
 * timer_start / timer_stop
 * ===================================================================== */
void timer_start(tim_t tmr)
{
    dev(tmr)->CTRL_bit.MODE = TMR_CTRL_MODE_Up;
}

void timer_stop(tim_t tmr)
{
    dev(tmr)->CTRL_bit.MODE = TMR_CTRL_MODE_Stop;
}
