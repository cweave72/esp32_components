print("---- running test")
a = {}

for i=1, 10 do
   a[i] = 5*i
end

sum = 0
for item, value in ipairs(a) do
   sum = sum + value
   print(string.format("[%u]: sum=%u", item, sum))
end

len = #a
print("The length of array is ", len)
print("The average is ", sum / len)
print("The sum is ", sum)
print("----")
