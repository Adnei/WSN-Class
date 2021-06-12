TIMEOUT(1200000); //20min
// TIMEOUT(120000);  // 2min
log.log("Time|ID|Message|LISTEN|TRANSMIT\n")
while (true) {
    msgArray = msg.split('|')
    if(msgArray.length > 1){
      if((msgArray[0]).split(':').length > 1 && (msgArray[1]).split(':').length > 1){
        listen = (msgArray[0]).split(':')[1]
        transmit = (msgArray[1]).split(':')[1]
        log.log(time + "|" + id + "|NA|"+listen+"|"+transmit+"\n")
      }
    }else{
      log.log(time + "|" + id + "|" + msg+"|NA|NA\n")
    }

    YIELD(); /* wait for another mote output */
}
