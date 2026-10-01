; Counts down from 10 to 1 (same program as in src/main.c).
; Only usable once the assembler exists (v3).

        push 10
loop:   print
        push 1
        sub
        dup
        jz end
        jmp loop
end:    halt
