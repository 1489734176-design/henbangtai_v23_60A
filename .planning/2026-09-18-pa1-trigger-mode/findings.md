# Findings

- Existing uncommitted source changes are in USER/board.c, USER/inc/board.h, USER/inc/parameter.h and USER/user_control.c; existing Keil outputs are also modified. Preserve other work.
- TRG_DIGITAL_EN already defaults to 1 in the compiled parameter header.
- Digital trigger detection reads PA1 low as released and high as pressed; digital duty assigns MAX_TOOL_DUTY to pwm_duty_aim_per. ADC path retains clamped analog mapping.
- Bsp_Adc_Init configures PA1 input pulldown in digital mode, whereas Bsp_Gpio_Init has an added input pullup configuration. Initialization order needs checking; eliminate conflicting duplicate configuration.
- Function comments still describe ADC-only trigger handling and the wrong debounce count.
- Bsp_Gpio_Init calls Bsp_Adc_Init first and then overwrites PA1 with pullup. Remove that later override and keep pulldown for the existing high-active digital logic.
- ADC scan uses nine fixed ranks and publishes VR along with BEMF/current/voltage/temperature. Preserve scan layout/timing; digital trigger and duty code must not consume VR. The user request concerns trigger detection, not redesigning motor ADC sampling.
- user_gears_handle's active branch transfers pwm_duty_aim_per directly to pwm_duty_aim. Existing fixed MAX_TOOL_DUTY assignment therefore reaches the target without bypassing the motor state machine.
