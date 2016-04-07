close all; clear all; clc;
# Analyze Contiguity of T7 Stream Data
fs = 1000; # Scan Frequency [Hz]
dat = load ("testing2channels");
ttl = dat(:,2);
plot(ttl)
# Count the number of point above and below the threshold on col 3
hi = sum(ttl>(3.3/2));
lo = sum(ttl<(3.3/2));
# Calculate the frequency of the of the square TTL pulse train and compare with the prescribed value
freak = ttl - (3.3/2);
inder = 0;
# threshold crossings
for p = 2:numel(freak)
  if freak(p) > 0 & freak(p-1) < 0
    inder = inder + 1;
    frind(inder) = p;
  end
end

dind = diff(frind);
T = mean(dind)/fs;
f = 1/T

# Checks should show that I have a contiguous dataset.

# Push the limit on the sample rate, etc.