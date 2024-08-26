local timer = require("timer")

print("---- running test")

local a = {}

for i=1, 10 do
   a[i] = 5*i
end

-- Create a new timer for tic/toc functions.
local t = timer.new()

print("setting timer")
timer.set_ms(t, 10000)
timer.tic(t)
repeat
    print(string.format("elapsed: %u ms ", timer.toc(t)))
    timer.threadsleep_ms(1000)
until timer.test(t)
print("timer expired")

timer.tic(t)
local sum = 0
for item, value in ipairs(a) do
   sum = sum + value
   print(string.format("[%u]: sum=%u", item, sum))
   timer.sleep_ms(200)
end
local elapsed = timer.toc(t)
print(string.format("elapsed = %.4f ms", elapsed/1000))

print("The length of array is ", #a)
print("The average is ", sum / #a)
print("The sum is ", sum)
print("----")
