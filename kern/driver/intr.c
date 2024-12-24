#include <intr.h>
  // LAB9 EXERCISE3: YOUR CODE
//You should check the manual to complete the function
/* intr_enable - enable irq interrupt */
void
intr_enable(void) {
	//__csrxchg(, , );
	// 清除IE位以启用中断
	__csrxchg(LISA_CSR_CRMD, LISA_CSR_CRMD_IE, 0);
}

/* intr_disable - disable irq interrupt */
void
intr_disable(void) {
	//__csrxchg(, , );
	// 设置IE位以禁用中断
	__csrxchg(LISA_CSR_CRMD, LISA_CSR_CRMD_IE, LISA_CSR_CRMD_IE);
}

