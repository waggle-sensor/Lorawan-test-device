function decodeUplink(input) {
  var bytes = input['bytes'];
  var decoded = {
    measurements: []
  };

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

  return { data: decoded };
}
