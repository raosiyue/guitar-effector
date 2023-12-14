t = [1:1/8192:1000];
input = 0.1 * sin(t);
output = -1 + 2 ./ (1 + exp(-100*input));

plot(input, output);