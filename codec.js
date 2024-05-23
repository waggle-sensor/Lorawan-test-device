function decodeUplink(input) {
    var bytes = input['bytes'];
    var decoded = {
      measurements:[]
    }
      
    var s_ch = bytes[0]; 	//Channel
    var s_type = bytes[1];	//Message Type
    var value = bytes[2];
    
    //follow payload format for lorawan-listener
    decoded.measurements.push({
      name: "lorawan.test",
      value: value
      });
    
    return {data: decoded};
}