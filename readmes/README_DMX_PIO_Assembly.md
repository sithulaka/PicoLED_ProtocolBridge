# DMX PIO Assembly Analysis - Complete Low-Level Breakdown

## **Generated PIO Assembly Code**

```cpp
static const uint16_t DmxOutput_program_instructions[] = {
    0xf035, //  0: set    x, 21           side 0     
    0x0741, //  1: jmp    x--, 1                 [7] 
    0xbf42, //  2: nop                    side 1 [7] 
            //     .wrap_target
    0x9fa0, //  3: pull   block           side 1 [7] 
    0xf327, //  4: set    x, 7            side 0 [3] 
    0x6001, //  5: out    pins, 1                    
    0x0245, //  6: jmp    x--, 5                 [2] 
            //     .wrap
};
```

## **Complete Instruction Analysis**

### **Instructions 0-2: BREAK and MAB Generation**

#### **Instruction 0: `0xf035` - BREAK Start**

**Binary Breakdown:**
```
0xf035 = 1111 0000 0011 0101

Bit Layout (PIO instruction format):
15-13: 111   = SET instruction (opcode)
12:    1     = Side-set present
11-10: 00    = Side-set value (0 = pin LOW)
9-5:   00110 = SET target (X register)
4-0:   10101 = SET data (21 decimal)
```

**Assembly Equivalent:**
```asm
set x, 21    side 0
```

**Execution:**
1. **X register** ← 21 (0x00000015)
2. **Side-set pin** ← 0 (DMX line goes LOW)
3. **Cycles**: 1 cycle
4. **PC** ← PC + 1 (advance to instruction 1)

#### **Instruction 1: `0x0741` - BREAK Loop**

**Binary Breakdown:**
```
0x0741 = 0000 0111 0100 0001

Bit Layout:
15-13: 000   = JMP instruction
12-8:  01110 = Delay (7 cycles) 
7-5:   100   = JMP condition (X-- != 0)
4-0:   00001 = JMP address (1)
```

**Assembly Equivalent:**
```asm
jmp x--, 1    [7]
```

**Execution Loop:**
```
Loop iteration 1: X=21 → 20, jump to PC=1, wait 7+1=8 cycles
Loop iteration 2: X=20 → 19, jump to PC=1, wait 8 cycles  
Loop iteration 3: X=19 → 18, jump to PC=1, wait 8 cycles
...
Loop iteration 21: X=1 → 0, jump to PC=1, wait 8 cycles
Loop iteration 22: X=0 → 0xFFFFFFFF, DON'T jump, wait 8 cycles
```

**Total BREAK Time:**
- **22 iterations** × **8 cycles each** = **176 cycles**
- **At 1MHz**: 176μs (exceeds DMX minimum of 88μs) ✓

#### **Instruction 2: `0xbf42` - MAB (Mark After Break)**

**Binary Breakdown:**
```
0xbf42 = 1011 1111 0100 0010

Bit Layout:
15-13: 101   = NOP instruction  
12:    1     = Side-set present
11-10: 11    = Side-set value (1 = pin HIGH)
9-8:   11    = NOP variant
7-3:   11110 = Delay (7 cycles)
2-0:   010   = Reserved/unused
```

**Assembly Equivalent:**
```asm
nop    side 1    [7]
```

**Execution:**
1. **No operation** (NOP)
2. **Side-set pin** ← 1 (DMX line goes HIGH)
3. **Wait** 7 + 1 = **8 cycles**
4. **PC** ← PC + 1

**MAB Timing:**
- **8 cycles** = **8μs at 1MHz** (meets DMX minimum) ✓

### **Instructions 3-6: Byte Transmission (The 11-bit Framing)**

#### **Instruction 3: `0x9fa0` - Pull Byte from FIFO**

**Binary Breakdown:**
```
0x9fa0 = 1001 1111 1010 0000

Bit Layout:
15-13: 100   = PULL instruction
12:    1     = Side-set present  
11-10: 11    = Side-set value (1 = pin HIGH)
9:     1     = IfEmpty behavior (block/stall)
8:     1     = Block flag
7-3:   11110 = Delay (7 cycles)
2-0:   000   = Reserved
```

**Assembly Equivalent:**
```asm
pull block    side 1    [7]
```

**Execution:**
1. **Check TX FIFO**: If empty, stall until DMA provides data
2. **OSR** ← next byte from FIFO (32-bit, byte in lower 8 bits)
3. **Side-set pin** ← 1 (keep DMX line HIGH)
4. **Wait** 7 + 1 = **8 cycles**

**What This Creates:**
- **After MAB, Before First Byte**: MAB (8μs HIGH) → Instruction 3 (8μs HIGH) → START bit of first byte
- **Between Subsequent Bytes**: Last data bit → Instruction 3 (8μs HIGH) → START bit of next byte

**This 8μs HIGH period serves as:**
- **STOP bits** for the previous byte (if any)
- **Inter-byte spacing** 
- **Line conditioning** before the next start bit

#### **Instruction 4: `0xf327` - START Bit + Setup Counter**

**Binary Breakdown:**
```
0xf327 = 1111 0011 0010 0111

Bit Layout:
15-13: 111   = SET instruction
12:    1     = Side-set present
11-10: 00    = Side-set value (0 = pin LOW)  
9-5:   11001 = SET target (X register)
4-3:   01    = Delay (3 cycles)
2-0:   111   = SET data (7)
```

**Assembly Equivalent:**
```asm
set x, 7    side 0    [3]
```

**Execution:**
1. **X register** ← 7 (bit counter for 8 data bits: 7,6,5,4,3,2,1,0)
2. **Side-set pin** ← 0 (DMX line goes LOW = START bit)
3. **Wait** 3 + 1 = **4 cycles** = **4μs START bit**

#### **Instruction 5: `0x6001` - Output Data Bit**

**Binary Breakdown:**
```
0x6001 = 0110 0000 0000 0001

Bit Layout:
15-13: 011   = OUT instruction
12-8:  00000 = No delay (0 cycles)
7-5:   000   = OUT destination (pins)
4-0:   00001 = OUT bit count (1 bit)
```

**Assembly Equivalent:**
```asm
out pins, 1
```

**Execution Details:**
1. **OSR shift operation**: 
   - Extract **LSB** (bit 0) from OSR
   - Shift OSR **right by 1 bit**
   - Output extracted bit to **pins**

**OSR Shift Example (for byte 0x7F = 01111111):**
```
Initial:     OSR = 0x0000007F = ...01111111
After bit 0: OSR = 0x0000003F = ...00111111, output = 1
After bit 1: OSR = 0x0000001F = ...00011111, output = 1  
After bit 2: OSR = 0x0000000F = ...00001111, output = 1
After bit 3: OSR = 0x00000007 = ...00000111, output = 1
After bit 4: OSR = 0x00000003 = ...00000011, output = 1
After bit 5: OSR = 0x00000001 = ...00000001, output = 1
After bit 6: OSR = 0x00000000 = ...00000000, output = 1
After bit 7: OSR = 0x00000000 = ...00000000, output = 0
```

**Timing:**
- **1 cycle** (no delay specified)
- **DMX pin** = extracted bit value

#### **Instruction 6: `0x0245` - Loop Control**

**Binary Breakdown:**
```
0x0245 = 0000 0010 0100 0101

Bit Layout:
15-13: 000   = JMP instruction
12-8:  00010 = Delay (2 cycles)
7-5:   010   = JMP condition (X-- != 0)  
4-0:   00101 = JMP address (5)
```

**Assembly Equivalent:**
```asm
jmp x--, 5    [2]
```

**Loop Execution:**
```
Iteration 1: X=7→6, jump to PC=5, wait 2+1=3 cycles, output bit 0
Iteration 2: X=6→5, jump to PC=5, wait 3 cycles, output bit 1  
Iteration 3: X=5→4, jump to PC=5, wait 3 cycles, output bit 2
Iteration 4: X=4→3, jump to PC=5, wait 3 cycles, output bit 3
Iteration 5: X=3→2, jump to PC=5, wait 3 cycles, output bit 4
Iteration 6: X=2→1, jump to PC=5, wait 3 cycles, output bit 5
Iteration 7: X=1→0, jump to PC=5, wait 3 cycles, output bit 6
Iteration 8: X=0→0xFFFFFFFF, DON'T jump, wait 3 cycles, output bit 7
```

**Per-Bit Timing:**
- **Instruction 5**: 1 cycle (out pins, 1)
- **Instruction 6**: 3 cycles (jmp with [2] delay)
- **Total**: 4 cycles = **4μs per data bit**

## **Side-Set vs OSR Output**

### **Important Distinction:**

**Side-Set Bits:**
- **Hardcoded** in the instruction itself
- **Immediate** pin control when instruction executes
- **Independent** of any shift register data
- **Controls framing bits** (BREAK, MAB, START bits, STOP bits)

**OSR Output:**
- **Data-driven** from shift register contents  
- **Dynamic** based on loaded data
- **Shifted out** bit by bit via `out pins, 1`
- **Controls only the 8 data bits**

### **Pin Control Summary:**

**Side-Set Controls:**
- **BREAK signal** (instruction 0: `side 0`)
- **MAB signal** (instruction 2: `side 1`) 
- **Inter-byte gaps** (instruction 3: `side 1`)
- **START bits** (instruction 4: `side 0`)

**OSR Controls:**  
- **Data bits only** (instruction 5: `out pins, 1`)

## **Complete Data Flow Path**

```
RAM → DMA → PIO FIFO → OSR → GPIO Pin
 ^      ^      ^        ^       ^
 |      |      |        |       |
 |      |      |        |    out pins,1
 |      |   pull block  |
 |   DMA writes         |
DMA reads              OSR shift
```

### **Step-by-Step Data Journey:**

1. **DMA** automatically copies bytes from `dmx_data[]` array to **PIO FIFO**
2. **`pull block`** transfers one byte from **FIFO** to **OSR** (32-bit register)
3. **`out pins, 1`** shifts bits **LSB-first** from **OSR** to **GPIO pin**
4. **Process repeats** for all 513 bytes automatically

## **Complete Byte Transmission Timeline**

### **For byte 0x7F (01111111 binary, transmitted as 11111110 LSB-first):**

```
Time:   0    4    8   12   16   20   24   28   32   36   40   44μs
       |    |    |    |    |    |    |    |    |    |    |    |
Pin:   |‾‾‾‾|____|‾‾‾‾|‾‾‾‾|‾‾‾‾|‾‾‾‾|‾‾‾‾|‾‾‾‾|____|‾‾‾‾‾‾‾‾|
       ^    ^    ^    ^    ^    ^    ^    ^    ^    ^         ^
       |    |    |    |    |    |    |    |    |    |         |
   Inter-  START D0=1 D1=1 D2=1 D3=1 D4=1 D5=1 D6=1 D7=0  Inter-
   byte    bit  LSB                              MSB     byte
   gap          (transmitted first)         (transmitted last) gap
```

## **Performance Summary**

### **BREAK Sequence:**
- **176 cycles** (176μs) LOW signal

### **MAB:**  
- **8 cycles** (8μs) HIGH signal

### **Per Byte:**
- **8 cycles** inter-byte gap (instruction 3)
- **4 cycles** START bit (instruction 4) 
- **32 cycles** data bits (8 × 4 cycles each)
- **Total: 44 cycles** (44μs) per byte

### **Overall Timing:**
- **Baud rate**: 250,000 bps ✓
- **Frame rate**: ~44Hz for 512 channels ✓
- **DMX compliant**: All timing requirements met ✓

This PIO program is a highly optimized, cycle-accurate implementation of the DMX512 protocol using the RP2040's programmable I/O system!
