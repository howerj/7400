
#ruledef instruction
{
; each instruction is 16 bits
; where X is the remaining 12 bits...
; i is an inaccessible internal register 
    
    loadm   =>  0b0000      ; a = (X)
    loadp   =>  0b0001      ; i=(X), a = (i)
    storep  =>  0b0010      ; i=(X), (i) = a
    load    =>  0b0011      ; a = X
    store   =>  0b0100      ; (X) = a
    add     =>  0b0101      ; a = a + (X)     --  Z
    sub     =>  0b0110      ; a = (X) - a     --  Z
    and     =>  0b0111      ; a = a and (X)   --  Z
    or      =>  0b1000      ; a = a or (X)    --  Z
    xor     =>  0b1001      ; a = a xor (X)   --  Z
    lsr     =>  0b1010      ; a shift right 1 --  Z 
    jmp     =>  0b1011      ; PC = X
    jmpZ    =>  0b1100      ; PC = X if Z
;    spare   =>  0b1101      ; 
    jsr     =>  0b1110      ; Stores PC at address X and jumps to X+1
    jmpi    =>  0b1111      ; PC = (X) can be used as return for jsr
    ; halt jump to same address sim will detect it or just hang real hardware
}

#ruledef
{
    {ins: instruction} {value: i12}   =>  ins    @   value
}

#bankdef ram
{
    #bits 16
    #addr 0x00
    #size 0x400
    #outp 0
}

#bank ram
