        push 3
        push 4
        call sumsq 2     ; -> 25
        print
        halt

sumsq:  load 0           ; a * a + b * b
        load 0
        mul
        load 1
        load 1
        mul
        add
        ret