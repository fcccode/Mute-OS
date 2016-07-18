; pages64.inc
;

[bits 32]

%include "page.inc"

;-----------------------------------------
; void clear_4K_page()：清4K页面
; input:  
;                esi: address
;------------------------------------------        
clear_4k_page:
clear_4K_page:
        pxor xmm1, xmm1
        mov eax, 4096
clear_4K_page_loop:        
        movdqu [esi + eax - 16], xmm1
        movdqu [esi + eax - 32], xmm1
        movdqu [esi + eax - 48], xmm1
        movdqu [esi + eax - 64], xmm1
        movdqu [esi + eax - 80], xmm1
        movdqu [esi + eax - 96], xmm1
        movdqu [esi + eax - 112], xmm1
        movdqu [esi + eax - 128], xmm1
        sub eax, 128
        jnz clear_4K_page_loop
        ret

;----------------------------------
; clear_4K_pages()：清数个4K页
; input:
;                esi: address,        edi: number
;----------------------------------
clear_4k_pages:
clear_4K_pages:
        test edi, edi
        jz clear_4K_pages_done
clear_4K_pages_loop:        
        call clear_4K_page
        add esi, 0x1000
        dec edi
        jnz clear_4K_pages_loop        
clear_4K_pages_done:        
        ret        
        

;---------------------------------------------------------------
; init_page(): 初始化 long mode 的页结构
; 描述：
;        在进入 long-mode 模式之前进行页表初始化
;----------------------------------------------------------------        

bsp_init_page:
init_page:


; 设置 PML4T 表，PML4T 的地址在 100000H
        mov DWORD [100000h], 101000h | RW | US | P                  ; PML4T[0]
        mov DWORD [100004h], 0

        ;; 由 0FFFFFF8X_XXXXXXXXX 的 virutal address 均是用户不可访问
        mov DWORD [100000h + 1FFh * 8], 102000h | RW | P     ; PML4T[0x1ff]
        mov DWORD [100000h + 1FFh * 8 + 4], 0

        mov DWORD [100000h + 0FFh * 8], 200000h | RW | US | P
        mov DWORD [100000h + 0FFh * 8 + 4], 0

; 设置 PDPT 表， 第 0 个 PDPT 表在 101000H，第 511 个 PDPT 表在 102000H
        mov DWORD [101000h], 103000h | RW | US | P                 ; PDPT[0] for PML4T[0]
        mov DWORD [101004h], 0

        ; 为了 00000000_FFE00000 - 00000000_FFE01FFFh 而映射
        mov DWORD [101000h + 3 * 8], 110000h | RW | US | P
        mov DWORD [101000h + 3 * + 4], 0

        mov DWORD [102000h + 1E0h * 8], 104000h | RW | P      ; PDPT[0x1e0] for PML4T[0x1ff]
        mov DWORD [102000h + 1E0h * 8 + 4], 0
        ; 0FFFFFFFF_CXXXXXXX ...
        mov DWORD [102000h + 1FFh * 8], 109000h | RW | P
        mov DWORD [102000h + 1FFh * 8 + 4], XD
        
        mov DWORD [200000h + 1FCh * 8], 201000h | RW | US | P
        mov DWORD [200000h + 1FCh * 8 + 4], 0
        mov DWORD [200000h + 1FFh * 8], 202000h | RW | US | P
        mov DWORD [200000h + 1FFh * 8 + 4], 0

; set PDT
        mov DWORD [103000h], 105000h | RW | US | P                 ; PDT[0] for PDPT[0] for PML4T[0]
        mov DWORD [103004h], 0

        ; virtual address 100000h - 2FFFFFh 映射到 100000h - 2FFFFFh 上
        ; 不可执行，用户不可访问
        ; 系统数据区

        mov DWORD [103000h + 1 * 8], 100000h | PS | RW | P
        mov DWORD [103000h + 1 * 8 + 4], XD

        mov DWORD [103000h + 2 * 8], 107000h | RW | P
        mov DWORD [103000h + 2 * 8 + 4], XD

        ; virutal address 800000h - 9FFFFFh 映射到 0FEE00000h - 0FEFFFFFFh（2M页面）
        ; 不可执行，用户不可访问，PCD = PWT = 1
        ; 用于 local APIC 区域
        mov DWORD [103000h + 4 * 8], 0C00000h | PCD | PWT | PS | RW | P
        mov DWORD [103000h + 4 * 8 + 4], XD

        ; PDT[0] for PDPT[0x1e0] for PML4T[0x1ff]
        mov DWORD [104000h], 106000h | RW | P
        mov DWORD [104004h], 0
        mov DWORD [104000h + 80h * 8], 108000h | RW | P
        mov DWORD [104000h + 80h * 8 + 4], 0

        ;*
        ;* 64-bit 模式下的 stack pointer 区域
        ;* 从 0FFFFFFFF_FFE00000 - 0FFFFFFFF_FFFFFFFF
        ;* 使用 2M 页映射到 0A00000h 地址
        ;* 使用于 kernel stack, IDT stack, sysenter stack, syscall stack ...
        ;*
        mov DWORD [109000h + 1FFh * 8], 0A00000h | PS | RW | P
        mov DWORD [109000h + 1FFh * 8 + 4], XD

        ; virutal address 00007FFF_00000000h - 00007FFF_001FFFFFh（2M页）
        ; 映射到物理地址 800000h
        ; 可执行，64 位用户代码执行区域
        mov DWORD [201000h], 800000h | PS | RW | US | P
        mov DWORD [201004h], 0
        mov DWORD [202000h + 1FFh * 8], 203000h | RW | US | P
        mov DWORD [202000h + 1FFh * 8 + 4], XD

        ;*
        ;* compatibility mode 下与 64-bit 对应的 stack pointer 区域
        ;* 从 00000000_FFE00000 - 00000000_FFFFFFFFh
        ;* 使用 2M 页映射到 600000h 地址
        ;* 
        mov DWORD [110000h + 1FFh * 8], 600000h | PS | RW | US | P
        mov DWORD [110000h + 1FFh * 8 + 4], XD

; set PT
        ; virutal address 0 - 0FFFh 映射到物理地址 0 - 0FFFh 上（4K页）
        ; no present!（保留未映射）
        mov DWORD [105000h + 0 * 8], 0000h | RW | US | P
        mov DWORD [105000h + 0 * 8 + 4], 0

        ; virtual address 0B000 - 0BFFFh 映射到物理地址 0B000 - 0BFFFh 上(4K页）
        ; r/w = u/s = p = 1
        mov DWORD [105000h + 0Bh * 8], 0B000h | RW | US | P
        mov DWORD [105000h + 0Bh * 8 + 4], 0
        
        ; virtual address 9000h - 0FFFFh 映射到物理地址 09000h - 0FFFFh 上
        mov DWORD [105000h + 09h * 8], 09000h | RW | US | P
        mov DWORD [105000h + 09h * 8 + 4], 0
        mov DWORD [105000h + 0Ah * 8], 0A000h | RW | US | P
        mov DWORD [105000h + 0Ah * 8 + 4], 0
        mov DWORD [105000h + 0Bh * 8], 0B000h | RW | US | P
        mov DWORD [105000h + 0Bh * 8 + 4], 0
        mov DWORD [105000h + 0Ch * 8], 0C000h | RW | US | P
        mov DWORD [105000h + 0Ch * 8 + 4], 0
        mov DWORD [105000h + 0Dh * 8], 0D000h | RW | US | P
        mov DWORD [105000h + 0Dh * 8 + 4], 0
        mov DWORD [105000h + 0Eh * 8], 0E000h | RW | US | P
        mov DWORD [105000h + 0Eh * 8 + 4], 0
        mov DWORD [105000h + 0Fh * 8], 0F000h | RW | US | P
        mov DWORD [105000h + 0Fh * 8 + 4], 0

        ; virtual address 10000h - 18FFFh 映射到物理地址 10000h - 18FFFh上（8共个4K页）
        ; 可执行，r/w = u/s = p = 1
        ; 用于 long.asm 模块执行空间
        mov DWORD [105000h + 10h * 8], 10000h | RW | US | P
        mov DWORD [105000h + 10h * 8 + 4], 0
        mov DWORD [105000h + 11h * 8], 11000h | RW | US | P
        mov DWORD [105000h + 11h * 8 + 4], 0
        mov DWORD [105000h + 12h * 8], 12000h | RW | US | P
        mov DWORD [105000h + 12h * 8 + 4], 0
        mov DWORD [105000h + 13h * 8], 13000h | RW | US | P
        mov DWORD [105000h + 13h * 8 + 4], 0
        mov DWORD [105000h + 14h * 8], 14000h | RW | US | P
        mov DWORD [105000h + 14h * 8 + 4], 0
        mov DWORD [105000h + 15h * 8], 15000h | RW | US | P
        mov DWORD [105000h + 15h * 8 + 4], 0
        mov DWORD [105000h + 16h * 8], 16000h | RW | US | P
        mov DWORD [105000h + 16h * 8 + 4], 0
        mov DWORD [105000h + 17h * 8], 17000h | RW | US | P
        mov DWORD [105000h + 17h * 8 + 4], 0
        mov DWORD [105000h + 18h * 8], 18000h | RW | US | P
        mov DWORD [105000h + 18h * 8 + 4], 0
        mov DWORD [105000h + 20h * 8], 20000h | RW | US | P
        mov DWORD [105000h + 20h * 8 + 4], 0


        ; virtual address 0B8000h - 0B9FFFh 映射到物理地址 0B8000h - 0B9FFFh 上（2个4K页）
        ; 不可执行，r/w = u/s = p = 1
        ; 用于 video 区域
        mov DWORD [105000h + 0B8h * 8], 0B8000h | RW | US | P
        mov DWORD [105000h + 0B8h * 8 + 4], XD
        mov DWORD [105000h + 0B9h * 8], 0B9000h | RW | US | P
        mov DWORD [105000h + 0B9h * 8], XD

        ; virutal address 0xfffffff800000000 - 0xfffffff800001fff (2 个 4K 页）
        ; 映射到物理地址 410000 - 411FFFh 上
        ; 不可执行，用户不可访问
        ; kernel 数据区
        mov DWORD [106000h], 410000h | RW | P
        mov DWORD [106004h], XD
        mov DWORD [106000h + 8], 411000h | RW | P
        mov DWORD [106000h + 8 + 4], XD


        ; virtual address 0FFFFFFF8_10000000h - 0FFFFFFF8_10001FFFh（2个4K页）
        ; 映射到物理地址 412000h - 412FFFh 上
        ; 用户不可访问
        ; kernel 执行区域
        mov DWORD [108000h], 412000h | RW | P
        mov DWORD [108004h], 0

        ; insterrupt IST1，不可执行
        mov DWORD [108000h + 1 * 8], 413000h | RW | P
        mov DWORD [108000h + 1 * 8 + 4], XD
        
        ; virutal address 00007FFF_FFE00000h - 00007FFF_FFE03FFFh(4个4K页）
        ; 映射到物理地址 607000h - 60AFFFh 上
        ; 用于 user stack 区
        mov DWORD [203000h], 414000h | RW | US | P                        ; 处理器 0
        mov DWORD [203000h + 4], 0
        mov DWORD [203000h + 1 * 8], 415000h | RW | US | P                ; 处理器 1
        mov DWORD [203000h + 1 * 8 + 4], 0
        mov DWORD [203000h + 2 * 8], 416000h | RW | US | P                ; 处理器 2
        mov DWORD [203000h + 2 * 8 + 4], 0
        mov DWORD [203000h + 3 * 8], 417000h | RW | US | P                ; 处理器 3
        mov DWORD [203000h + 3 * 8 + 4], 0

            ; virutal address 400000h 映射到物理地址 400000h 上（使用 4K 页）
        ; 不可执行，用户不可访问，用于 DS save 区域
        mov DWORD [107000h], 400000h | RW | P
        mov DWORD [107004h], XD
init_page_done:
        ret
        

        
;********* page table base *******
pml4t_base      dq PML4T_BASE
pdpt_base       dq 0
pdt_base        dq 0
pt_base         dq 0

maxphyaddr_select_mask  dq 0

pml4e_msg       db 'PML4E: base=0x', 0
pdpte_msg       db 'PDPTE: base=0x', 0
pde_msg         db 'PDE:   base=0x', 0
pte_msg         db 'PTE:   base=0x', 0
attr_msg        db 'Attr: ',0

not_available   db '***  not available (P=0)', 0
reserved_error  db '       <ERROR: reserved bit is not zero!>', 10, 0
                

;********* entry flags **********        

p_flags         db 'p',0
rw_flags        db 'r/w',0
us_flags        db 'u/s',0
pwt_flags       db 'pwt', 0
pcd_flags       db 'pcd', 0
a_flags         db 'a',0
d_flags         db 'd', 0
ps_flags        db 'ps',0
g_flags         db 'g',0
pat_flags       db 'pat',0
ignore_flags    db '-',0
blank_flags     db '   ', 0
reserved_flags  db '0', 0
xd_flags        db 'xd', 0

;************ flags ***********
pml4e_flags             dd xd_flags, blank_flags, ignore_flags, ignore_flags, ignore_flags, ignore_flags
                        dd reserved_flags, ignore_flags, a_flags, pcd_flags, pwt_flags, us_flags, rw_flags, p_flags, -1
                                
pdpte_long_1g_flags     dd xd_flags, pat_flags, ignore_flags, ignore_flags, ignore_flags, g_flags, ps_flags
                        dd d_flags, a_flags, pcd_flags, pwt_flags, us_flags, rw_flags, p_flags, -1
                                        
pdpte_long_flags        dd xd_flags, blank_flags, ignore_flags, ignore_flags, ignore_flags, ignore_flags                
                        dd ps_flags, ignore_flags, a_flags, pcd_flags, pwt_flags, us_flags, rw_flags, p_flags, -1

pde_long_2m_flags       dd xd_flags                                                
                        dd pat_flags, ignore_flags, ignore_flags, ignore_flags, g_flags, ps_flags
                        dd d_flags, a_flags, pcd_flags, pwt_flags, us_flags, rw_flags, p_flags, -1
                                
pde_long_4k_flags       dd xd_flags        
                        dd blank_flags, ignore_flags, ignore_flags, ignore_flags, ignore_flags, ps_flags
                        dd ignore_flags, a_flags, pcd_flags, pwt_flags, us_flags, rw_flags, p_flags, -1
                                        
pte_long_4k_flags       dd xd_flags
                        dd blank_flags, ignore_flags, ignore_flags, ignore_flags, g_flags, pat_flags
                        dd d_flags, a_flags, pcd_flags, pwt_flags, us_flags, rw_flags, p_flags, -1