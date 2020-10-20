// Copy this into a "custom" Payload Format at thethingsnetwork.org
function Decoder(bytes, port)
{
	var decoded = {};

	decoded.version = bytes[0];

	if (decoded.version == 1)
	{
		decoded.pm25 = (bytes[2] << 8) | bytes[1];
		decoded.pm10 = (bytes[4] << 8) | bytes[3];
	}

	return decoded; 
}