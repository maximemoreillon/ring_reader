/*
BCC=LEN XOR Status XOR DATA
*/

#define STX 0x02
#define ETX 0x03

#define CMD_LENGTH 1 // CMD is Always 1
#define STATUS_LENGTH 1 // Status is awlays 1

#define PARA_RF_BIT 5
#define RF32 false

#define READER_TIMEOUT 1000

#define WORD_LENGTH 4 // One word is 4 bytes, i.e. 32 bits

void send_packet(char payload[], int payload_length){
  // Sends a command packet to the reader

  // Flush Serial in case there was a response halfway through
  Serial1.flush();
 
//  Serial1.enableTx(true);
  for(int i=0; i<payload_length; i++){
    Serial1.write(payload[i]);
  }
//  Serial1.enableTx(false);
}

byte format_para(byte address){
  // Formats the PARA byte

  byte output = 0x00;

  // Address is the 4 last bits of PARA
  byte address_truncated = address & 0x0f;

  output |= address_truncated;

  // In case of RF 32 communication, bit 5 must be set to 1
  if(RF32) output |= (1 << 1);
 
  return output;
  
}

void build_packet_and_send( byte cmd, char data[], byte data_len ){

  /*
   * Packet format (original);
   * STX(0x02)+LEN(1byte)+CMD(1byte)+DATA(nbytes)+BCC(1byte)+ETX(0x03)
   * 
   * BCC computation:
   * BCC = LEN xor CMD xor DATA
   * 
   * Notes:
   * - In this implementation, DATA is combined with PARA
   */

  // 

  // Packet length = STX (1) + LEN (1) + CMD (1) + BCC (1) + ETX (1) + PARA (variable)
  byte packet_size = data_len + 5;

  // Defien the packet
  char packet[packet_size];
  
  packet[0] = STX;
  packet[1] = 1 + data_len;
  packet[2] = cmd;

  byte bcc_index = sizeof(packet)-2;

  // DATA
  for(int i=0; i<data_len; i++){
    packet[i+3] = data[i];
  }

  // BCC
  packet[bcc_index] = packet[1] ^ packet[2];
  for(int i=0; i<data_len; i++){
    packet[bcc_index] = packet[bcc_index] ^ packet[i+3];
  }

  // ETX
  packet[sizeof(packet)-1] = ETX;

  send_packet(packet, sizeof(packet));
}


void read_word(byte address){
  // Read a word at a specific address
  // 0x02+0x02+0x11+PARA(1byte) +BCC+0x03
  // PARA contains the address to read in the last 4 bits
  
  byte cmd = 0x11; // 0x11 is the command for reading
  char para[1];
  
  para[0] = format_para(address);

  build_packet_and_send(cmd, para, sizeof(para) );

}


void write_word( byte address, char word_to_write[], byte word_length ){
  /*
   * writes a word at a specific address
   * A word is 4 bytes, same length as an Arduino long
   * Format: 0x02+0x06+0x10+PARA(1byte)+DATA(4bytes)+BCC+0x03
   * 
   * Notes:
   * - Para and Data are combiend into a 5-byte PARA
   * - Para contaisn the address in 4 bits
   */

  byte cmd = 0x10;
  char para[5];
  
  para[0] = format_para(address);

  for(int i=0; i<word_length; i++){
    para[i+1] = word_to_write[i];
  }

  build_packet_and_send(cmd, para, sizeof(para));

}

void login( char password[] ){
  /*
   * Login
   * 
   * Para is password
   */
  //0x02+0x05+0x12+PASSWORD(4bytes)+BCC+0x03
  
  byte cmd = 0x12;

  build_packet_and_send(cmd, password, sizeof(password) );

}

void protection( char para[] ){

  // 0x02+0x05+0x13+PROTECTION(4bytes)+BCC+0x03
  
  byte cmd = 0x13;

  build_packet_and_send(cmd, para, sizeof(para) );

}

void disable(){

  // PROBABLY DANGEROUS!

  // 0x02+0x05+0x14+0xFF+0xFF +0xFF +0xFF +BCC+0x03
  
  byte cmd = 0x14;
  char para[WORD_LENGTH] = {0xFF,0xFF,0xFF,0xFF};

  build_packet_and_send(cmd, para, sizeof(para) );

}

void read_em4100(){
  /*
   * STX  + LEN  + CMD  + BCC + ETX
   * 0x02 + 0x01 + 0x81 + BCC + 0x03
   */
  
  byte cmd = 0x81;
  char para[0];

  build_packet_and_send(cmd, para, sizeof(para) );

}



long byte_array_to_long(char byte_array[], byte byte_array_length){
  long output;

  for(int i=0; i<byte_array_length; i++){
    output |= byte_array[i] << (byte_array_length-1 - i)*8;
  }

  return output;
}

void long_to_char_array(char byte_array[], long input){

  for(int i=0; i<sizeof(input); i++){
    byte_array[i] = input >> (sizeof(input) -1 -i)*8;
  }

}




Response get_reader_response_sync() {

  /*
   * This is a synchronous function, i.e. it waits for the response to the sent command
   * Response format:
   * STX(0x02)+LEN(1byte)+Status(1byte)+DATA(nbytes)+BCC(1byte)+ETX(0x03)
   */
  
  boolean response_started = false;
  int response_byte_index = 0;

  Response response;
  
  long start_time = millis();
    
  while(true){

    // Timeout
    if(millis() - start_time > READER_TIMEOUT) break;
  
    // if no byte available, do nothing
    if (Serial1.available()) {
      
      // The current byte
      byte received_byte = (byte) Serial1.read();
  
      // A full response starts with STX (0x02)
      // Warning: DATA may also contain a 0x02
      if(!response_started && received_byte == STX){
    
        // Acknowledge start
        response_started = true;
        
      }
    
      if(response_started){
    
        // Second byte (index 1) sets the response length
        if(response_byte_index == 1){
          response.length = received_byte - STATUS_LENGTH;
        }
    
        // 3rd byte sets the status
        if(response_byte_index == 2){
          
          /*
          0x0 --- success
          0x1 --- failure (Happens when no tag is presented)
          0x2 --- data error
          0x3 --- command error
          0x4 --- parameter errror
          */
          
          response.status = received_byte;
        }
        
        // Dealing with the data if any
        // Len = length of status (1) + length of data (N)
        // Check if there is data to read
        if(response.length){

          
          byte data_start_index = 3;
          byte data_end_index = data_start_index + response.length;
          byte data_index = response_byte_index - 3; // because STX, LEN and STATUS before
  
          // Store data in the array
          if(response_byte_index >= data_start_index && response_byte_index < data_end_index){
            response.data[data_index] = received_byte;
          }
        }
    
        // finding final byte
        // Adding STX LEN BCC and ETX
        // Exit the loop once finished
        boolean is_last_byte = response_byte_index == response.length +4;
        if(is_last_byte && received_byte == ETX){
          break;
        }
    
        response_byte_index ++;
      }
    }
  }

  // clear the serial buffer
  // Maybe not ideal or necessary
  Serial1.flush();

  
   
  return response;
}

Response read_word_sync( byte address ){
  read_word(address);
  delay(5);
  return get_reader_response_sync();
}

Response read_em4100_sync(){
  read_em4100();
  delay(5);
  return get_reader_response_sync();
}

Response login_sync( char password[] ){
  login(password);
  delay(5);
  return get_reader_response_sync();
}

Response write_word_sync( byte address, char word_to_write[], byte word_length ){
  write_word(address, word_to_write, word_length);
  delay(5);
  return get_reader_response_sync();
}


int compare_em4100(char target[]) {
  /*
   * -1: Read error
   * 0: OK
   * 1: Wrong
   */
  Response response = read_em4100_sync();
  if(response.status) return -1;

//  for(int i=0; i<response.length; i++){
//    Serial.print(response.data[i],HEX);
//    Serial.println("");
//  }
  
  if(strncmp(target,response.data,response.length) == 0) return 0;
  return 1;
}


boolean write_word_and_verify_sync( byte address, char word_to_write[], byte word_length ){
  write_word_sync(address, word_to_write, word_length);
  delay(10);
  Response response = read_word_sync(address);
  if(response.status) return false;
  return strncmp(word_to_write, response.data, word_length) == 0;

}
