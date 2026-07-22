An event-driven Side-Channel Analysis (SCA) suite designed to capture nano-scale power and electromagnetic (EM) cryptographic leaks.
Originally inspired by the chaotic, unrepeatable dynamics of natural systems, this architecture isolates chaotic sub-pixel and sub-microvolt
"peaks" above a filtered baseline threshold. Instead of flooding bandwidth with continuous raw data streams, it utilizes a hardware-triggered,
dual-core Circular Burst Buffer Pipeline to capture precise temporal execution windows.

# Install core mathematical dependencies

pip install numpy scipy

# Execute the integrated verification simulation pipeline

python -m tests.simulate_pipeline
📈 Mathematical Processing Pipelines🛡️ Eliminating Phase Drift (Jitter Alignment)Target chips utilize internal clock variation to mitigate SCA. This suite aligns traces using windowed cross-correlation arrays against a baseline anchor trace before processing:pythoncross_corr = np.correlate(trace[:half], reference_trace[:half], mode='same')
shift = np.argmax(cross_corr) - (half // 2)
aligned_trace = np.roll(trace, -shift)
🧮 Hardware Leakage Verification (Hamming Distance Model)Instead of static Hamming Weights, this engine models dynamic power usage of switching transistors across a data bus, processing state changes cleanly through an AES-128 S-Box transition vector:\(\text{Leakage\ Hypothesis}=\text{HammingWeight}(\text{SBox}(\text{Input}\oplus \text{KeyCandidate})\oplus \text{Input})\)⚠️ High-Frequency Target Note (1 GHz+ Targets)When evaluating high-performance SoC application chips (e.g., Cortex-A, Raspberry Pi), physical GPIO lines introduce propagation latency. For these systems:Use an external RF H-Field Near-Field Probe positioned directly above the target silicon packaging.Route the output signal into a high-speed comparator array linked to the ESP32-S3 external interrupt vector to lock down temporal window parameters.
