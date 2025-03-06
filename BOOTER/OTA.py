import serial
import time
import os
import struct
import threading
import sys

# Bảng CRC32 MPEG-2 từ firmware ESP32
CRC_TABLE = [
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039, 0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
    0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF, 0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4,
]

def calculate_crc32(data):
    """Tính CRC32 MPEG-2 theo đúng cách STM32 xử lý"""
    
    checksum = 0xFFFFFFFF
    crc_packet = bytearray()  

    for byte in data:
        crc_packet.extend([0x00, 0x00, 0x00, byte])

    for byte in crc_packet:
        top = (checksum >> 24) & 0xFF
        top ^= byte
        checksum = ((checksum << 8) & 0xFFFFFFFF) ^ CRC_TABLE[top]

    return checksum

def list_bin_files(directory="."):
    """List all .bin files in the given directory."""
    return [f for f in os.listdir(directory) if f.endswith(".bin")]

def select_bin_file():
    """Display available .bin files and allow the user to select one."""
    bin_files = list_bin_files()

    if not bin_files:
        print("No .bin files found! Please enter the file path manually.")
        return input("Enter the firmware file path (.bin): ").strip()

    print("\nAvailable firmware files:")
    for idx, file in enumerate(bin_files, start=1):
        print(f"{idx}. {file}")

    while True:
        choice = input("Select a file by entering its number (or 0 to enter a file path): ").strip()

        if choice == "0":
            return input("Enter the firmware file path (.bin): ").strip()
        elif choice.isdigit() and 1 <= int(choice) <= len(bin_files):
            return os.path.join(os.getcwd(), bin_files[int(choice) - 1])
        else:
            print("Invalid choice, please try again.")

class STM32Bootloader:
    def __init__(self, port, baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.serial = None
        self.firmware_number = None  # Store firmware selection
        self.NOT_ACKNOWLEDGE = 0xAB
        self.JUMP_SUCCEEDED = 0x01
        self.ERASE_SUCCEEDED = 0x03
        self.WRITE_SUCCEEDED = 0x01
        self.RESET_SUCCEEDED = 0x01

    def connect_serial(self):
        """Connect to the STM32 bootloader via UART"""
        try:
            self.serial = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                bytesize=serial.EIGHTBITS,
                timeout=2
            )
            print(f"Connected to STM32 on {self.port}")
            return True
        except serial.SerialException as e:
            print(f"Serial connection error: {e}")
            return False

    def send_packet(self, packet, response_length, time_out):
        """Send a packet and wait for response"""
        self.serial.write(packet)
        start_time = time.time()
        while time.time() - start_time < time_out:  # Timeout
            if self.serial.in_waiting >= response_length:
                return self.serial.read(response_length)
        return None

    def read_chip_id(self):
        """Read Chip ID (0x10)"""
        packet = bytearray(6)
        packet[0] = 5
        packet[1] = 0x10
        crc = calculate_crc32(packet[:2])
        struct.pack_into('<I', packet, 2, crc)
        response = self.send_packet(packet, 2, 1)
        if response and response[0] != self.NOT_ACKNOWLEDGE:
            chip_id = (response[1] << 8) | response[0]
            print(f"Chip ID: 0x{chip_id:04X}")
            return chip_id
        print("Failed to read Chip ID")
        return None
    
    def jump_to_boot(self):
        """Send 'reset' command to bootloader to jump to boot mode"""
        packet = b"reset\r"  # Use a proper byte string
        self.send_packet(packet, 0, 0)
        return None
    
    def check(self):
        """Send 'alive_check' command to bootloader to jump to boot mode"""
        packet = b"alive_check\r"  # Use a proper byte string
        self.send_packet(packet, 0, 0)
        return None

    def select_firmware(self):
        """Select firmware number (1 or 2)"""
        while True:
            choice = input("Select firmware (1 or 2): ")
            if choice in ["1", "2"]:
                self.firmware_number = int(choice)
                print(f"Firmware {self.firmware_number} selected")
                return
            print("Invalid input! Please enter 1 or 2.")

    def erase_flash(self):
        """Erase Flash (0x13)"""
        if not self.firmware_number:
            print("No firmware selected! Please choose firmware first.")
            return
        print(f"Erasing firmware {self.firmware_number}...")
        packet = bytearray(7)
        packet[0] = 6
        packet[1] = 0x13
        if self.firmware_number == 0x01:
            address = 0x08008000 
            packet[2] = 1
        elif self.firmware_number == 0x02:
            address = 0x08080000 
            packet[2] = 2
        else:
            return False
        crc = calculate_crc32(packet[:3])
        struct.pack_into('<I', packet, 3, crc)
        print(f"Erasing at 0x{address:08X}")
        response = self.send_packet(packet, 1, 20)
        if response and response[0] == self.ERASE_SUCCEEDED:
            print("Erase Flash Successful")
            return True
        print("Erase Flash Fail")
        return False

    def upload_application(self, bin_path):
        """Upload firmware via UART"""
        if not self.firmware_number:
            print("No firmware selected! Please choose firmware (1 or 2) first.")
            return
        if not os.path.exists(bin_path):
            print(f"File not found: {bin_path}")
            return

        print(f"Uploading firmware {self.firmware_number} ({len(open(bin_path, 'rb').read())} bytes)...")
        with open(bin_path, 'rb') as f:
            firmware_data = f.read()

        if self.firmware_number == 0x01:
            address = 0x08008000
        elif self.firmware_number == 0x02:
            address = 0x08080000   
        else:
            return False

        print(f"Begin upload at 0x{address:08X}")
        file_size = len(firmware_data)
        chunk_size = 128
        base_address = address  

        total_frames = (file_size + chunk_size - 1) // chunk_size  
        frame_index = 0

        print(f"Firmware-Size: {file_size} bytes, Total frames: {total_frames}")

        for i in range(0, file_size, chunk_size):
            actual_chunk_size = min(chunk_size, file_size - i)  # Độ dài dữ liệu thực tế
            packet_length = actual_chunk_size + 15  # 15 = 11 (header) + 4 (CRC)

            packet = bytearray(packet_length)
            packet[0] = packet_length - 1  # Độ dài gói (trừ đi byte này)
            packet[1] = 0x14  # Mã lệnh Upload Application
            address = base_address + (0x80 * frame_index)
            struct.pack_into('<I', packet, 2, address)
            packet[6] = actual_chunk_size  # Số byte dữ liệu thực tế

            struct.pack_into('<H', packet, 7, frame_index)  
            struct.pack_into('<H', packet, 9, total_frames)  

            chunk = firmware_data[i:i + actual_chunk_size]
            packet[11:11 + actual_chunk_size] = chunk  

            crc =calculate_crc32(packet[:11 + actual_chunk_size])  
            struct.pack_into('<I', packet, 11 + actual_chunk_size, crc)

            print(f"Send frame {frame_index + 1}/{total_frames} Address: 0x{address:08X}, Size: {actual_chunk_size} bytes")

            response = self.send_packet(packet, 1, 5)
            if not response or response[0] != self.WRITE_SUCCEEDED:
                print("Failed to write block!")
                return False

            progress = (i + actual_chunk_size) / file_size * 100
            print(f"Process: {progress:.1f}%")
            frame_index += 1

        print("Upload Application Successful!")
        return True

    def jump_to_application(self):
        """Jump to application (0x12)"""
        if not self.firmware_number:
            print("❌ No firmware selected! Please choose firmware first.")
            return
        print(f"Jumping to firmware {self.firmware_number}...")

        packet = bytearray(10)
        packet[0] = 9  # Độ dài gói tin
        packet[1] = 0x12  # Lệnh Jump
        if self.firmware_number == 0x01:
            address = 0x08008000
        elif self.firmware_number == 0x02:
            address = 0x08080000   
        else:
            return False
        packet[2] = (address >> 24) & 0xFF
        packet[3] = (address >> 16) & 0xFF
        packet[4] = (address >> 8) & 0xFF
        packet[5] = address & 0xFF
        
        crc = calculate_crc32(packet[:6])  # CRC trên 6 byte đầu
        struct.pack_into('<I', packet, 6, crc)  # Nhúng CRC vào packet
        print(f"Jumping to 0x{address:08X}")
        response = self.send_packet(packet, 1, 1)
        if response and response[0] == self.JUMP_SUCCEEDED:
            print("Jump to Application Successful")
            return True
        print("Jump to Application Fail")
        return False
    
    def reset_chip(self):
        """Reset STM32 chip (0x15)"""
        print("Resetting STM32 chip...")
        packet = bytearray(6)
        packet[0] = 5
        packet[1] = 0x15
        crc = calculate_crc32(packet[:2])
        struct.pack_into('<I', packet, 2, crc)
        response = self.send_packet(packet, 1, 1)
        if response and response[0] != self.NOT_ACKNOWLEDGE:
            print("STM32 Reset Successful")
            return True
        print("STM32 Reset Failed")
        return False

    def uart_terminal(self):
        """Real-time UART terminal mode"""
        if not self.serial:
            print("UART is not connected!")
            return

        print("\nTerminal Mode - Type to send, press Ctrl+C to exit\n")

        def read_uart():
            """Thread to continuously read UART data"""
            try:
                while True:
                    if self.serial.in_waiting:
                        data = self.serial.read(self.serial.in_waiting).decode(errors='ignore')
                        print(f"{data.strip()}", end="", flush=True)
                    time.sleep(0.1)
            except KeyboardInterrupt:
                print("\nExiting terminal mode...")

        uart_thread = threading.Thread(target=read_uart, daemon=True)
        uart_thread.start()

        try:
            while True:
                command = input("> ")  # Get user input
                if command.lower() == "exit":
                    break  # Exit terminal mode
                self.send_packet(command.encode() + b"\r\n", 0, 0)  # Send input over UART
        except KeyboardInterrupt:
            print("\nExiting terminal mode...")

    def close(self):
        """Close the serial connection"""
        if self.serial and self.serial.is_open:
            self.serial.close()
            print("Serial connection closed")

def main():
    print("Welcome to STM32 Bootloader Menu")
    port = input("Enter COM port (e.g., COM5 or /dev/ttyUSB0): ")
    
    bootloader = STM32Bootloader(port)
    if not bootloader.connect_serial():
        return

    bin_file = None  # Firmware file path

    while True:
        try:
            print("\nMenu Options:")
            print("1: Check Connection")  # New option
            print("2: Read Chip ID")
            print("3: Select Firmware (1 or 2)")
            print("4: Erase Flash")
            print("5: Select Firmware File")
            print("6: Flash Firmware")
            print("7: Jump to Application")
            print("8: Listen to UART")
            print("9: Reset Chip")  # New option
            print("10: Exit")
            

            choice = input("Select an option (1-10): ")
            
            if choice == "1":
                bootloader.check()
                bootloader.uart_terminal()         
            elif choice == "2":
                bootloader.read_chip_id()
            elif choice == "3":
                bootloader.select_firmware()  # Select firmware
            elif choice == "4":
                bootloader.erase_flash()
            elif choice == "5":
                bin_file = select_bin_file()
                if os.path.exists(bin_file):
                    print(f"Firmware file selected: {bin_file}")
                else:
                    print("File not found! Please try again.")
                    bin_file = None
            elif choice == "6":
                if bin_file:
                    bootloader.upload_application(bin_file)
                else:
                    print("No firmware file selected! Please select a file first.")
            elif choice == "7":
                bootloader.jump_to_application()
            elif choice == "8":
                bootloader.uart_terminal()
            elif choice == "9":
                bootloader.jump_to_boot()
                bootloader.reset_chip()  # New reset function
            elif choice == "10":
                print("Exiting...")
                bootloader.close()
                break
            else:
                print("Invalid choice, please try again.")

        except KeyboardInterrupt:
                bootloader.close()
                print("\nKeyboard Interrupt detected. Exiting safely...")
                sys.exit(0)  # Exit safely
    

if __name__ == "__main__":
    main()
