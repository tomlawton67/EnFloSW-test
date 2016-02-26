-- Author : Kyle Gompertz
-- Last Edited : 11 November 2015
slave_address = {0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C}
zeros = {97,98,98,98,97,96,95,96}
cm = {0,0,0,0,0,0,0,0}
numBytesTX = 0
numBytesRX = 0
processing = 0
dataTX = {0x00,0x51}
dataRX = {0x00,0x00}
--Configure I2C Bus
MB.W(5100, 0, 3) --SDA
MB.W(5101, 0, 2) --SCL
MB.W(5102, 0, 65466) --I2C throttle = 65466 (24.3 kHz)
MB.W(5103, 0, 0) --I2C options
LJ.IntervalConfig(0, 20)
ps = 1
psend = 8
while true do
		if processing == 0 then	--Command : Range
			if ps == 1 then
	 			numBytesTX = 2
				numBytesRX = 0
				MB.W(5108, 0, numBytesTX)
				MB.W(5109, 0, numBytesRX)
				error_val = MB.WA(5120, 99, numBytesTX, dataTX)
			end
			MB.W(5104, 0, slave_address[ps])
			if LJ.CheckInterval(0) then
				MB.W(5110, 0, 1)
				if ps == psend then
					processing = 1
					ps = 1
				elseif ps < psend then
					ps = ps +1
				end
			end
		elseif processing == 1 then	--Command : Report Last Range Reading
			if ps == 1 then
				numBytesTX = 0
				numBytesRX = 2
				MB.W(5108, 0, numBytesTX)
				MB.W(5109, 0, numBytesRX)
			end
			MB.W(5104, 0, slave_address[ps])
			RAMreg = 46180 + ps;
			if LJ.CheckInterval(0) then
				MB.W(5110, 0, 1)
				processing = 2
			end
		elseif processing == 2 then	--Command : Read Register 5160
			if LJ.CheckInterval(0) then
				dataRX, error_val = MB.RA(5160, 99, numBytesRX)
				cm[ps] = zeros[ps] - (256*dataRX[1] + dataRX[2])
				if cm[ps] < 0 then
				  cm[ps] = 0
				end
				MB.W(RAMreg,0,cm[ps])
				if ps == psend then
					processing = 0
					ps = 1
					print(cm[1], cm[2], cm[3], cm[4], cm[5], cm[6], cm[7], cm[8])
				elseif ps < psend then
					processing = 1
					ps = ps + 1
				end
			end
		end
end