# ICMP Packet Creation and Parsing Program

## General Overview

### The Ping Command
The `ping` command is a network diagnostic tool that tests host reachability and measures round-trip time for packets. It works using the ICMP (Internet Control Message Protocol):
- Sends **ICMP Echo Request** packets (type 8) to the target host
- Waits for **ICMP Echo Reply** packets (type 0) in response
- Each packet contains an identifier and sequence number to match requests with replies
- Includes an optional payload and checksum for data integrity

### Code Functionality
This program demonstrates how to:
1. Create ICMP packets in a logical data structure
2. Serialize the structure into raw bytes (as they would be transmitted over the network)
3. Parse raw bytes back into the logical structure
4. Verify data integrity through the round-trip process

## Detailed Code Explanation

### Main Data Structures

```c
// Raw representation (network format)
struct raw_icmp {
    u8 type;         // ICMP type (8=echo, 0=reply)
    u8 code;         // Code (always 0 for echo/reply)
    u16 checksum;    // Checksum for integrity verification
    u16 identifier;  // Process or sender ID
    u16 sequence;   // Sequence number
    u8 payload[];   // Optional data
} packed;

// Logical representation (easier to use)
typedef struct {
    icmp_kind kind;       // Logical type (ICMP_ECHO or ICMP_REPLY)
    u16 identifier;       // Process ID
    u16 sequence;         // Sequence number
    u16 length;           // Payload length
    u8 *payload;          // Payload data
} icmp_packet;
```

### Utility Functions

1. **`dumphex`**:
   - Prints a buffer in hexadecimal format
   - Parameters: buffer, length, newline flag

2. **`clearbuf`**:
   - Zeroes out a buffer using memset

3. **`compute_checksum`**:
   - Calculates the ICMP checksum (16-bit one's complement sum)
   - Used to verify packet integrity

### ICMP Functions

1. **`create_icmp`**:
   - Creates a logical packet by allocating memory
   - Sets type, ID, sequence and payload
   - Example: `create_icmp(ICMP_ECHO, "Hello!", 6, 42, 1)`

2. **`serialize_icmp`**:
   - Converts the logical structure into raw bytes
   - Calculates and inserts the checksum
   - Aligns the packet to 2 bytes (required for checksum)

3. **`parse_icmp`**:
   - Converts raw bytes back into logical structure
   - Extracts header and payload
   - Maps numeric type to logical enumeration

4. **`display_icmp`**:
   - Displays packet contents in human-readable format

### Program Flow (main)

```c
int main() {
    // 1. Create ICMP Echo Request packet
    const char *msg = "Hello!";
    icmp_packet *pkt = create_icmp(ICMP_ECHO, (const u8 *)msg, strlen(msg), 42, 1);

    // 2. Show original packet
    printf("--- Original Packet ---\n");
    display_icmp(pkt);

    // 3. Serialize into raw bytes
    u16 raw_len;
    u8 *raw = serialize_icmp(pkt, &raw_len);
    printf("--- Serialized Bytes ---\n");
    dumphex(raw, raw_len, 1);

    // 4. Parse raw bytes back
    icmp_packet *parsed = parse_icmp(raw, raw_len);
    printf("--- Parsed Back ---\n");
    display_icmp(parsed);

    // 5. Clean up memory
    free(pkt->payload);
    free(pkt);
    free(raw);
    free(parsed->payload);
    free(parsed);

    return 0;
}
```

## Output Explanation

### 1. Original Packet
```
--- Original Packet ---
Kind:       Echo
Identifier: 42
Sequence:   1
Length:     6
Payload:    48 65 6c 6c 6f 21 
```
- **Kind**: Echo Request (type 8)
- **Identifier**: 42 (arbitrary ID)
- **Sequence**: 1 (first packet)
- **Length**: 6 bytes (length of "Hello!")
- **Payload**: "Hello!" in hexadecimal (48='H', 65='e', etc.)

### 2. Serialized Bytes
```
--- Serialized Bytes ---
08 00 a9 0c 2a 00 01 00 48 65 6c 6c 6f 21 
```
Raw packet structure:
- `08`: ICMP type (8 = Echo Request)
- `00`: Code
- `a9 0c`: Calculated checksum
- `2a 00`: Identifier (42 in little-endian)
- `01 00`: Sequence (1 in little-endian)
- `48 65 6c 6c 6f 21`: Payload "Hello!"

### 3. Parsed Packet
```
--- Parsed Back ---
Kind:       Echo
Identifier: 42
Sequence:   1
Length:     6
Payload:    48 65 6c 6c 6f 21 
```
- Identical to original packet, demonstrating:
  1. Correct serialization/parsing
  2. Valid checksum
  3. Intact data preservation

## Compilation and Execution

1. Save the code to a file (e.g., `icmp_demo.c`)
2. Compile with:
   ```bash
   gcc -o ping ping.c
   ```
3. Execute:
   ```bash
   ./ping
   ```

## Important Notes

1. This is an **educational demo** and doesn't send real packets
2. In a real implementation:
   - Numeric fields would use **network byte order** (big-endian)
   - Raw sockets would be used for actual sending/receiving
   - Checksum would be verified on received packets
3. The `packed` structure prevents compiler padding
4. Checksum ensures integrity but not security