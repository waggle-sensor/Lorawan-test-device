function decodeUplink(input) {
  var bytes = input['bytes'];
  var decoded = {
    measurements: []
  };

  // Skip known test packet: [1, 1, 3]
  if (!(bytes.length === 3 && bytes[0] === 1 && bytes[1] === 1 && bytes[2] === 3)) {

    var packetId = bytes[0];                    // First byte = Packet ID
    var packetSize = bytes[bytes.length - 1];   // Last byte = Packet size
  
    decoded.measurements.push({
      name: "packet_id",
      value: packetId
    });
  
    decoded.measurements.push({
      name: "packet_size",
      value: packetSize
    });
  
    decoded.measurements.push({
      name: "payload_length",
      value: bytes.length
    });
  }

  return { data: decoded };
}
