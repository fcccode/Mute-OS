/**
 * func.h
 * Global functions declarition
 */

PUBLIC void* memocpy(void* pDst, void* pSrc, int iSize);
PUBLIC void  print(const char* pszInfo, const char color);
PUBLIC void init_prot();
PUBLIC void init_8259A();
PUBLIC void out_byte(u16 port, u8 value);
PUBLIC u8 in_byte(u16 port);
PUBLIC void init_prot();
PUBLIC void exception_handler(int vec_no, unsigned int err_code);
PUBLIC char* itoa(char * str, int num);
PUBLIC void print_bit(int input, char color);
PUBLIC void init_8259A_asm();
PUBLIC void spurious_irq(int irq);  
