import numpy as np
from scipy.signal import butter, sosfiltfilt

class ProductionSideChannelSuite:
    def __init__(self, reference_voltage=3.3, adc_bit_resolution=16, sample_rate_hz=1e9):
        """
        Initializes an enterprise-grade side-channel analysis suite.
        Equipped with digital signal filtering, trace realignment, 
        and high-precision floating-point correlation engines.
        """
        self.v_ref = float(reference_voltage)
        self.resolution = int(adc_bit_resolution)
        self.max_adc_val = float((1 << self.resolution) - 1)
        self.sample_rate = float(sample_rate_hz)
        
        # AES-128 Forward SubBytes S-Box
        self.sbox = np.array([
            0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
            0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
            0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
            0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
            0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
            0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
            0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
            0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
            0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
            0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
            0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
            0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
            0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
            0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
            0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
            0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
        ], dtype=np.uint8)
        
        self.hw_table = np.array([bin(i).count('1') for i in range(256)], dtype=np.float32)

    def apply_bandpass_filter(self, traces, lowcut_hz=5e6, highcut_hz=200e6, order=5):
        """
        Applies a zero-phase digital Butterworth filter.
        Strips ambient RF noise and isolation-transformer harmonics.
        """
        nyquist = 0.5 * self.sample_rate
        low = lowcut_hz / nyquist
        high = highcut_hz / nyquist
        sos = butter(order, [low, high], btype='band', output='sos')
        
        filtered_traces = np.zeros_like(traces, dtype=np.float64)
        for i in range(traces.shape[0]):
            filtered_traces[i, :] = sosfiltfilt(sos, traces[i, :])
        return filtered_traces

    def align_trace_jitter_dtw(self, traces, reference_trace, max_shift=15):
        """
        Fast windowed cross-correlation alignment engine.
        Corrects micro-architectural CPU clock instability and trace jitter.
        """
        aligned_traces = np.copy(traces)
        num_traces, num_samples = traces.shape
        
        for i in range(num_traces):
            cross_corr = np.correlate(traces[i, :num_samples//2], reference_trace[:num_samples//2], mode='same')
            shift = np.argmax(cross_corr) - (num_samples // 4)
            
            if abs(shift) <= max_shift and shift != 0:
                aligned_traces[i, :] = np.roll(traces[i, :], -shift)
        return aligned_traces

    def generate_hamming_distance_hypotheses(self, plaintexts, byte_index=0):
        """
        Hamming Distance Model: HD(SBox(Input ^ Key) ^ Input)
        Models power consumption of data buses overwriting previous states.
        """
        num_traces = len(plaintexts)
        hypotheses = np.zeros((num_traces, 256), dtype=np.float32)
        target_bytes = plaintexts[:, byte_index]
        
        for k_guess in range(256):
            intermediate_state = self.sbox[target_bytes ^ k_guess]
            # XOR intermediate state with initial state to count bit flips
            bit_transitions = intermediate_state ^ target_bytes
            hypotheses[:, k_guess] = self.hw_table[bit_transitions]
            
        return hypotheses

    def compute_cpa_matrix(self, traces, hypotheses):
        """
        Vectorized Pearson Product-Moment Correlation calculation.
        Scales cleanly over thousands of trace rows and temporal points.
        """
        scaled_traces = (traces.astype(np.float64) / self.max_adc_val) * self.v_ref
        
        traces_centered = scaled_traces - np.mean(scaled_traces, axis=0)
        hypotheses_centered = hypotheses - np.mean(hypotheses, axis=0)
        
        sum_sq_traces = np.sum(traces_centered**2, axis=0)
        sum_sq_hypotheses = np.sum(hypotheses_centered**2, axis=0)
        
        numerator = np.dot(hypotheses_centered.T, traces_centered)
        denominator = np.sqrt(np.outer(sum_sq_hypotheses, sum_sq_traces))
        
        denominator[denominator == 0] = 1e-12
        return numerator / denominator

# --- ENTERPRISE END-TO-END VERIFICATION ---
if __name__ == "__main__":
    NUM_TRACES = 1200
    TIME_SAMPLES = 800
    TARGET_BYTE = 0
    SECRET_KEY_BYTE = 0xD3  # Key targeted for physical recovery
    
    print("[*] Deploying Production Side-Channel Analysis Suite...")
    suite = ProductionSideChannelSuite(reference_voltage=3.3, adc_bit_resolution=16)
    
    # 1. Generate Plaintext Arrays
    np.random.seed(1337)
    plaintexts = np.random.randint(0, 256, size=(NUM_TRACES, 16), dtype=np.uint8)
    
    # 2. Simulate Noisy Hardware Traces with Clock Jitter
    print("[*] Generating simulated analog hardware signals...")
    base_noise = np.random.normal(0, 0.5, size=(NUM_TRACES, TIME_SAMPLES))
    
    # Inject deliberate, microscopic Hamming Distance leakage at sample point 412
    intermediates = suite.sbox[plaintexts[:, TARGET_BYTE] ^ SECRET_KEY_BYTE] ^ plaintexts[:, TARGET_BYTE]
    leak_signals = np.array([suite.hw_table[x] for x in intermediates]) * 0.08
    base_noise[:, 412] += leak_signals
    
    # Convert simulation matrix to physical 16-bit ADC steps
    raw_unaligned_adc = ((base_noise - base_noise.min()) / (base_noise.max() - base_noise.min()) * 65535).astype(np.uint32)
    
    # Introduce clock stability jitter (random horizontal shifts)
    for i in range(NUM_TRACES):
        raw_unaligned_adc[i, :] = np.roll(raw_unaligned_adc[i, :], np.random.randint(-4, 5))
        
    # 3. Execute DSP Pipeline Stages
    print("[*] DSP Step 1: Realignment of clock-jitter via cross-correlation...")
    reference_frame = raw_unaligned_adc[0, :]
    aligned_adc = suite.align_trace_jitter_dtw(raw_unaligned_adc, reference_frame)
    
    print("[*] DSP Step 2: Processing digital zero-phase bandpass filtering...")
    filtered_adc = suite.apply_bandpass_filter(aligned_adc, lowcut_hz=1e6, highcut_hz=150e6)
    
    print("[*] DSP Step 3: Compiling dynamic Hamming Distance bus transition tables...")
    hypotheses = suite.generate_hamming_distance_hypotheses(plaintexts, byte_index=TARGET_BYTE)
    
    print("[*] DSP Step 4: Resolving complete vectorized correlation space...")
    cpa_matrix = suite.compute_cpa_matrix(filtered_adc, hypotheses)
    
    # 4. Parse Extracted Key
    absolute_cpa = np.abs(cpa_matrix)
    recovered_key = np.argmax(np.max(absolute_cpa, axis=1))
    leak_sample = np.argmax(absolute_cpa[recovered_key, :])
    peak_val = absolute_cpa[recovered_key, leak_sample]
    
    print("\n================ PRODUCTION ANALYTICS ================")
    print(f"Target Hardware Secret Byte : {hex(SECRET_KEY_BYTE).upper()}")
    print(f"Recovered Circuit Secret Byte: {hex(recovered_key).upper()}")
    print(f"Temporal Leak Target Point  : Clock Cycle {leak_sample}")
    print(f"Signal-to-Noise Peak Corr   : {peak_val:.6f}")
    print("======================================================")
