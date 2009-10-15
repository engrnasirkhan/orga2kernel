begin_page_directory:
;; |	   base | reserved  |pat|avl| g |ps | d | a |pcd|pwt|u/s|r/w| P |
;;  00_0000_0000_0_0000_0000___0_000___1___0___0___0___0___1___0___1___1b
dd 0x10b + begin_page_table
end_page_directory:
times (4096 - (end_page_directory-begin_page_directory)) db 0x0



begin_page_table:
;; mi kernel tiene 0x3040 bytes y arranca en 0x1200
;; asi si hago identity mapping necesito paginar en la 
;; tabla de paginas hasta 0x4240

;; |	   base        base |pat|avl| g |ps | d | a |pcd|pwt|u/s|r/w| P |
;;  00_0000_0000_0_0000_0000___0_000___1___0___0___0___0___1___0___1___1b

%assign i 0
%rep 6
     dd i<<12 | 0x0000010b
     %assign i i+1
%endrep
;;ahora en 0x6000 tengo la memoria de video
dd 0xb8000 | 0x0000010b
end_page_table:
times (4096 - (end_page_table-begin_page_table)) db 0x0

