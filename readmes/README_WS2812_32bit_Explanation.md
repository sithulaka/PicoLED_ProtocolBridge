# WS2812 32-bit vs 24-bit Explanation

## WS2812 Protocol vs PIO Implementation

### **WS2812 Native Format**
WS2812 LEDs indeed use **24 bits per LED**:
- **Green**: 8 bits (bits 23-16)
- **Red**: 8 bits (bits 15-8) 
- **Blue**: 8 bits (bits 7-0)

### **Why 32 Bits Are Sent**

The reason lies in how the **PIO state machine** processes the data:

```cpp
void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}
```

#### **The `<< 8u` Left Shift**

When the code does `pixel_grb << 8u`, it shifts the 24-bit color data 8 positions to the left:

```
Original 24-bit data:  [G7 G6 G5 G4 G3 G2 G1 G0][R7 R6 R5 R4 R3 R2 R1 R0][B7 B6 B5 B4 B3 B2 B1 B0]
After << 8u shift:     [G7 G6 G5 G4 G3 G2 G1 G0][R7 R6 R5 R4 R3 R2 R1 R0][B7 B6 B5 B4 B3 B2 B1 B0][0  0  0  0  0  0  0  0]
                       ↑----- 24 bits of color data -----↑ ↑--- 8 zero bits ---↑
```

#### **PIO State Machine Operation**

The PIO state machine (from `ws2812.pio.h`) is programmed to:

1. **Read 32 bits** from the FIFO
2. **Process only the upper 24 bits** for WS2812 timing
3. **Ignore the lower 8 bits** (which are zeros after the shift)

#### **Why This Design?**

This approach has several advantages:

1. **PIO Efficiency**: PIO state machines work most efficiently with 32-bit words
2. **Timing Precision**: The PIO program can use the upper bits for precise bit-banging
3. **Hardware Alignment**: 32-bit transfers align with ARM processor word size
4. **Future Extensibility**: The unused 8 bits could be used for additional features

### **PIO State Machine Logic**

The WS2812 PIO program typically works like this:

```assembly
; Simplified PIO assembly logic
.wrap_target
    out x, 1        ; Output 1 bit from shift register
    jmp !x, do_zero ; Jump if bit is 0
do_one:
    set pins, 1     ; HIGH for T0H time
    ; ... timing delays ...
    set pins, 0     ; LOW for T0L time
    jmp .wrap_target
do_zero:
    set pins, 1     ; HIGH for T1H time  
    ; ... timing delays ...
    set pins, 0     ; LOW for T1L time
.wrap
```

The PIO processes each bit sequentially, converting digital bits into precisely-timed WS2812 pulses.

### **Memory Layout Visualization**

```
32-bit word sent to PIO:
┌─────────────────────────────────┐
│ G7 G6 G5 G4 G3 G2 G1 G0 │ Bit 31-24 (Green)
│ R7 R6 R5 R4 R3 R2 R1 R0 │ Bit 23-16 (Red)  
│ B7 B6 B5 B4 B3 B2 B1 B0 │ Bit 15-8  (Blue)
│ 0  0  0  0  0  0  0  0  │ Bit 7-0   (Unused)
└─────────────────────────────────┘
                          ↑
                    These 8 bits are ignored
```

### **Auto-Pull Mechanism**

The critical line that explains the 32-bit vs 24-bit question is:

```c
sm_config_set_out_shift(&c, false, true, rgbw ? 32 : 24);
```

This configures the **Output Shift Register (OSR)** with:
- `false`: Shift direction (MSB first)
- `true`: Auto-pull enabled 
- `rgbw ? 32 : 24`: **Threshold for auto-pull**

## **How the Auto-Pull Works**

### **For Standard WS2812 (RGB) - 24-bit threshold:**

1. **CPU sends 32-bit word** via `pio_sm_put_blocking()`
2. **PIO OSR accumulates bits** until 24 bits are collected
3. **Auto-pull triggers** when OSR has 24 bits
4. **8 unused bits remain** in the FIFO for next operation

### **Data Flow Visualization:**

```
CPU sends:     [G7 G6 G5 G4 G3 G2 G1 G0][R7 R6 R5 R4 R3 R2 R1 R0][B7 B6 B5 B4 B3 B2 B1 B0][00 00 00 00 00 00 00 00]
               ↑________________________ 32 bits total ________________________↑

PIO OSR pulls: [G7 G6 G5 G4 G3 G2 G1 G0][R7 R6 R5 R4 R3 R2 R1 R0][B7 B6 B5 G4 G3 G2 G1 G0]
               ↑_________________ 24 bits used _________________↑

Remaining:     [00 00 00 00 00 00 00 00] ← 8 bits discarded/unused
```

### **Alternative Implementations**

Some implementations might:
1. Pack multiple LEDs into fewer 32-bit words
2. Use different bit arrangements for RGBW LEDs
3. Utilize the unused bits for brightness control or metadata

This design pattern is common in embedded systems where hardware efficiency and timing precision are crucial. The "waste" of 8 bits per LED is negligible compared to the benefits of simplified PIO programming and precise timing control.
