timer = require("timer")

print("---- running test")

a = {}

for i=1, 10 do
   a[i] = 5*i
end

timer.tic()
sum = 0
for item, value in ipairs(a) do
   sum = sum + value
   print(string.format("[%u]: sum=%u", item, sum))
   timer.sleep_ms(200)
end
elapsed = timer.toc()
print(string.format("elapsed = %.4f ms", elapsed/1000))

print("The length of array is ", #a)
print("The average is ", sum / #a)
print("The sum is ", sum)
print("----")
