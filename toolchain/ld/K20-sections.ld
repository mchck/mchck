.pin_hooks :
{
    pin_hooks_A = . ;
    *(SORT(.pin_hooks.PIN_PTA*))
    pin_hooks_A_end = . ;

    pin_hooks_B = . ;
    *(SORT(.pin_hooks.PIN_PTB*))
    pin_hooks_B_end = . ;

    pin_hooks_C = . ;
    *(SORT(.pin_hooks.PIN_PTC*))
    pin_hooks_C_end = . ;

    pin_hooks_D = . ;
    *(SORT(.pin_hooks.PIN_PTD*))
    pin_hooks_D_end = . ;

    KEEP(*(.pin_hooks.*))
} > rom
