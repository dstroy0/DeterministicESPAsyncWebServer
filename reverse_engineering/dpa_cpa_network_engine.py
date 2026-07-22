import socket
import struct
import numpy as np
import threading
from queue import Queue

class SideChannelStreamReceiver:
    def __init__(self, host='0.0.0.0', port=8080, sample_points=1000):
        """
        Hosts a non-blocking TCP socket to ingest triggered trace packets 
        directly from the ESP sensor node.
        """
        self.host = host
        self.port = port
        self.sample_points = sample_points
        self.packet_queue = Queue(maxsize=5000)
        self.running = False
        
        # Packet format expected from ESP: 
        # [4 Bytes: Trace ID] [16 Bytes: AES Plaintext Data] [sample_points * 2 Bytes: Raw 16-bit ADC samples]
        self.header_format = "<I16s"
        self.header_size = struct.calcsize(self.header_format)
        self.trace_data_size = self.sample_points * 2 # 16-bit unsigned shorts
        self.total_packet_size = self.header_size + self.trace_data_size

    def start(self):
        self.running = True
        self.server_thread = threading.Thread(target=self._listen_loop, daemon=True)
        self.server_thread.start()
        print(f"[*] Stream receiver live on {self.host}:{self.port}. Awaiting ESP connection...")

    def _listen_loop(self):
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind((self.host, self.port))
        server_socket.listen(1)
        
        while self.running:
            try:
                conn, addr = server_socket.accept()
                conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1) # Disable Nagle's algorithm for low latency
                print(f"[+] Sensor node connected from {addr}")
                
                buffer = bytearray()
                while self.running:
                    data = conn.recv(65536)
                    if not data:
                        break
                    buffer.extend(data)
                    
                    # Parse out fully completed packets from the stream buffer
                    while len(buffer) >= self.total_packet_size:
                        packet = buffer[:self.total_packet_size]
                        del buffer[:self.total_packet_size]
                        
                        # Unpack header
                        trace_id, plaintext = struct.unpack(self.header_format, packet[:self.header_size])
                        # Unpack raw ADC values into numpy array directly from bytes
                        raw_adc = np.frombuffer(packet[self.header_size:], dtype=np.uint16)
                        
                        # Push to thread-safe processing queue
                        if not self.packet_queue.full():
                            self.packet_queue.put((trace_id, np.frombuffer(plaintext, dtype=np.uint8), raw_adc))
                            
                conn.close()
            except Exception as e:
                print(f"[-] Connection dropped or error encountered: {e}")

    def gather_batch(self, batch_size=1000):
        """
        Blocks until a specific batch size is accumulated, then yields the matrix
        ready to be passed straight to the ProductionSideChannelSuite CPA engine.
        """
        plaintexts_matrix = []
        traces_matrix = []
        
        print(f"[*] Accumulating {batch_size} physical traces from queue...")
        while len(traces_matrix) < batch_size:
            trace_id, plaintext, raw_adc = self.packet_queue.get()
            plaintexts_matrix.append(plaintext)
            traces_matrix.append(raw_adc)
            
        return np.array(plaintexts_matrix, dtype=np.uint8), np.array(traces_matrix, dtype=np.uint32)

    def stop(self):
        self.running = False

# --- HOW TO RUN THE LIVE INTEGRATION ---
if __name__ == "__main__":
    # 1. Start network ingestion engine
    receiver = SideChannelStreamReceiver(port=8080, sample_points=1000)
    receiver.start()
    
    try:
        # 2. Block until ESP has streamed 1,200 trace capture events
        plaintexts, raw_traces = receiver.gather_batch(batch_size=1200)
        print(f"[+] Matrix Built! Ingested {raw_traces.shape[0]} traces with {raw_traces.shape[1]} samples.")
        
        # 3. Pass directly to  filtering and CPA processing loop...
        # suite = ProductionSideChannelSuite(adc_bit_resolution=16)
        # aligned = suite.align_trace_jitter_dtw(raw_traces, raw_traces[0])
        # ... execute attack
    except KeyboardInterrupt:
        receiver.stop()
