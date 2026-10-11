        push 5
        call fact 1
        print
        halt

fact:   dup
        jz base
        dup
        push 1
        sub
        call fact 1
        mul
        ret

base:   push 1
        ret