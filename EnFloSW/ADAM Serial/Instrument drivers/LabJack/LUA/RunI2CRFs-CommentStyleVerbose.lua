-- Author : Kyle Gompertz
-- Last Edited : 11 November 2015
-- This version should be identical to RunI2CRFs.lua but with more comments. Comments are 
-- removed from the other version to minimize the memory occupied on the T7 flash chip.
-- RunI2CRFs.lua is opened in Kipling's Lua Script Debugger, saved to the flash chip, and
-- set to Run @ Powerup.

-- Slave addresses
-- The default address.dec for the RFs is 224 (112 w/o RW bit)
-- RFs have been configured with address.dec = 202:2:216
-- the slave_address array contains the 8 address.hex values w/o RW bit
slave_address = {0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C}
zeros = {97,98,96,98,97,96,95,95} -- "zero" RF readings
cm = {0,0,0,0,0,0,0,0} -- initialize roof height values array

-- Initialize I2C Data
numBytesTX = 0
numBytesRX = 0
processing = 0
dataTX = {0x00,0x51} -- Range Command
dataRX = {0x00,0x00} -- initialize Read Bytes

--Configure I2C Bus
MB.W(5100, 0, 3) -- SDA - I2C Data line (FIO3)
MB.W(5101, 0, 2) -- SCL - I2C Clock line (FIO2)
MB.W(5102, 0, 65466) -- I2C throttle: throttle = 0 (~450 kHz); throttle = 65516 (93.6 kHz); throttle = 65466 (24.3 kHz)
MB.W(5103, 0, 0) -- I2C options

-- Only one operation is performed within each loop. First, all RFs are sent a Range
-- Command. Then, each RF is sent a Report Reading Command and Read LabJack Register
-- Command. The Report and Read Commands are both executed before moving onto the next RF. 
LJ.IntervalConfig(0, 20) -- should the #RFs be < 8; this will need adjustment
ps = 1 -- start index
psend = 8 -- end index
while true do

		if processing == 0 then	-- Command : Range
			if ps == 1 then
	 			numBytesTX = 2
				numBytesRX = 0
				MB.W(5108, 0, numBytesTX) -- bytes to transmit
				MB.W(5109, 0, numBytesRX) -- bytes to receive
				error_val = MB.WA(5120, 99, numBytesTX, dataTX) -- load I2C TX data
			end
			MB.W(5104, 0, slave_address[ps]) -- set I2C slave address
			if LJ.CheckInterval(0) then -- interval completed
				MB.W(5110, 0, 1) -- I2C Go
				if ps == psend then -- move on to next operation
					processing = 1
					ps = 1
				elseif ps < psend then -- send range command to all RFs
					ps = ps +1
				end
			end
			
		elseif processing == 1 then	-- Command : Report Last Range Reading
			if ps == 1 then
				numBytesTX = 0
				numBytesRX = 2
				MB.W(5108, 0, numBytesTX) -- bytes to transmit
				MB.W(5109, 0, numBytesRX) -- bytes to receive
			end
			MB.W(5104, 0, slave_address[ps]) -- set I2C slave address
			RAMreg = 46180 + ps; -- USER_RAM#(1:8) = 46181:1:46188
			if LJ.CheckInterval(0) then
				MB.W(5110, 0, 1) -- I2C Go
				processing = 2
			end
			
		elseif processing == 2 then	--  Command : Read Register 5160
			if LJ.CheckInterval(0) then
				dataRX, error_val = MB.RA(5160, 99, numBytesRX) -- Read Register 5160
				cm[ps] = zeros[ps] - (256*dataRX[1] + dataRX[2]) -- Join bytes -- > UINT16
				if cm[ps] < 0 then -- unsigned 16-bit value should be > 0 (re: ram)
				  cm[ps] = 0
				end
				MB.W(RAMreg,0,cm[ps]) -- store height value in unsigned 16-bit ram register
				if ps == psend then -- go back to Range Command
					processing = 0
					ps = 1
					-- Useful to display scan values when opening Lua Script Debugger
					print(cm[1], cm[2], cm[3], cm[4], cm[5], cm[6], cm[7], cm[8])
				elseif ps < psend then -- go back to Report Command
					processing = 1
					ps = ps + 1
				end
			end
		end
end