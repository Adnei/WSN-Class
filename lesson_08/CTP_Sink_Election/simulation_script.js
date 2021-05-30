TIMEOUT(1200000); //20min
// TIMEOUT(120000);  // 2min
log.log("Time|ID|Message|LISTEN|TRANSMIT\n")
while (true) {
    if(msg.contains("REMOVE SINK NOW")){
        mote.getSimulation().removeMote(mote)
    }
    // powertrace.c is modified in order to print only listen and trasmit
    // the printf separes listen and transmit by |
    msgArray = msg.split('|')
    if(msgArray.length > 1){
      if((msgArray[0]).split(':').length > 1 && (msgArray[1]).split(':').length > 1){
        listen = (msgArray[0]).split(':')[1]
        transmit = (msgArray[1]).split(':')[1]
        log.log(time + "|" + id + "| |"+listen+"|"+transmit+"\n")
      }
    }else{
      log.log(time + "|" + id + "|" + msg+"|NA|NA\n")
    }

    YIELD(); /* wait for another mote output */
}
