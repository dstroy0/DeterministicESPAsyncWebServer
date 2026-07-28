================================================================================
ESPRESSIF ESP32-P4: EXHAUSTIVE CRYPTOGRAPHIC REGISTRAR & IOP STANDARDS
================================================================================

--------------------------------------------------------------------------------
1. AES CO-PROCESSOR REGISTER MAP (Base Address: 0x60010000)
--------------------------------------------------------------------------------
Offset | Register Name         | R/W | Bitfield / Hardware Specification
-------|-----------------------|-----|------------------------------------------
0x000  | AES_KEY_0_REG         | W   | [31:0] Key Word 0 (Total 8 words up to 0x01C for AES-256)
0x020  | AES_TEXT_IN_0_REG     | W   | [31:0] Data Input Word 0 (Total 4 words up to 0x02C)
0x030  | AES_TEXT_OUT_0_REG    | R   | [31:0] Data Output Word 0 (Total 4 words up to 0x03C)
0x040  | AES_MODE_REG          | R/W | Bit [0]: 0=Encrypt, 1=Decrypt

       |                       |     | Bits [2:1]: Key Size (00=128-bit, 01=192-bit, 10=256-bit)
       |                       |     | Bits [5:3]: Mode (000=ECB, 001=CBC, 010=OFB, 011=CTR, 100=GCM)
0x044  | AES_ENDIAN_REG        | R/W | Bits [5:0]: Hardware-level Endian inversion for In/Out/Key
0x048  | AES_TRIGGER_REG       | W   | Bit [0]: Strobe high (1) to start manual calculation
0x04C  | AES_STATE_REG         | R   | Bits [1:0]: 0=IDLE, 1=LOAD, 2=CALC, 3=DONE (Busy flags)
0x050  | AES_IV_0_REG          | W   | [31:0] Initialization Vector Word 0 (Total 4 words up to 0x05C)
0x060  | AES_H_0_REG           | W   | [31:0] GCM GHASH Key Word 0 (Total 4 words up to 0x06C)
0x070  | AES_J_0_REG           | W   | [31:0] GCM J0 Counter Word 0 (Total 4 words up to 0x07C)
0x080  | AES_DPA_NICK_REG      | R/W | [31:0] Differential Power Analysis side-channel signal mask
0x084  | AES_DMA_IN_CTRL_REG   | R/W | Bit [0]: Enable direct 2D-DMA master input channel link
0x088  | AES_DMA_OUT_CTRL_REG  | R/W | Bit [0]: Enable direct 2D-DMA master output channel link

--------------------------------------------------------------------------------
2. SHA CO-PROCESSOR REGISTER MAP (Base Address: 0x60012000)
--------------------------------------------------------------------------------
Offset | Register Name         | R/W | Bitfield / Hardware Specification
-------|-----------------------|-----|------------------------------------------
0x000  | SHA_MODE_REG          | R/W | Bits [2:0]: 0=SHA-1, 1=SHA-224, 2=SHA-256, 3=SHA-384, 4=SHA-512
0x004  | SHA_START_REG         | W   | Bit [0]: Pulse 1 to run initial state/first message block
0x008  | SHA_CONTINUE_REG      | W   | Bit [0]: Pulse 1 to process subsequent buffered block data
0x00C  | SHA_BUSY_REG          | R   | Bit [0]: 1=Engine currently running hash calculations, 0=Ready
0x010  | SHA_H_0_REG           | R   | [31:0] Internal Digest State registers (Total 16 words up to 0x04C)
0x050  | SHA_M_0_REG           | W   | [31:0] Raw FIFO Message Block Input space (Total 16 words up to 0x08C)

--------------------------------------------------------------------------------
3. RSA / MULTI-PRECISION INTEGER (MPI) REGISTER MAP (Base Address: 0x60014000)
--------------------------------------------------------------------------------
Offset | Register Name         | R/W | Bitfield / Hardware Specification
-------|-----------------------|-----|------------------------------------------
0x000  | RSA_SET_START_MODEXP  | W   | Bit [0]: Execute Montgomery Modular Exponentiation
0x004  | RSA_SET_START_MODMULT | W   | Bit [0]: Execute Montgomery Modular Multiplication
0x008  | RSA_SET_START_MULT    | W   | Bit [0]: Execute Large-Integer Multiplication (Normal)
0x00C  | RSA_QUERY_BUSY_REG    | R   | Bit [0]: 1=RSA execution active, 0=Idle
0x010  | RSA_LENGTH_REG        | R/W | Bits [6:0]: Matrix Bounds calculation configuration: (WordCount - 1)
0x014  | RSA_COMP_MODE_REG     | R/W | Bit [0]: 0=Standard mode, 1=Accelerated Montgomery Core
0x800  | RSA_MEM_X_BASE        | R/W | Array map to Multiplicand Buffer X (512 bytes / 4096-bit max)
0xA00  | RSA_MEM_Y_BASE        | R/W | Array map to Exponent/Multiplier Buffer Y (512 bytes)
0xC00  | RSA_MEM_M_BASE        | R/W | Array map to Modulus Buffer M (512 bytes)
0xE00  | RSA_MEM_Z_BASE        | R/W | Array map to Result/Intermediate Buffer Z (512 bytes)

--------------------------------------------------------------------------------
4. ECDSA / ECC CORE ACCELERATOR REGISTER MAP (Base Address: 0x60016000)
--------------------------------------------------------------------------------
Offset | Register Name         | R/W | Bitfield / Hardware Specification
-------|-----------------------|-----|------------------------------------------
0x000  | ECC_START_REG         | W   | Bit [0]: Write 1 to launch the hardware calculation sequence
0x004  | ECC_BUSY_REG          | R   | Bit [0]: 1=Elliptic Curve engine active, 0=Pipeline open
0x008  | ECC_MODE_REG          | R/W | Bits [2:0]: Curve Select (0=P-256, 1=P-192, 2=P-384, 3=P-521)

       |                       |     | Bits [5:3]: Macro operation command code (Scalar Mult, etc)
0x00C  | ECC_INT_EN_REG        | R/W | Bit [0]: Assert global interrupt flag upon completion
0x010  | ECC_INT_CLR_REG       | W   | Bit [0]: Clear completion status latch
0x100  | ECC_POINT_X_BASE      | R/W | Hardware Parameter Buffer for point coordinate X (128 words)
0x180  | ECC_POINT_Y_BASE      | R/W | Hardware Parameter Buffer for point coordinate Y (128 words)
================================================================================
