
function fre = frequncy_get(this_frame)
scale_out = 0.48828125 * 2;
%figure;
%plot(this_frame);
window_size = floor(40 /( 0.48828125 * 2));
valid_pt = [];
for i = window_size + 1:length(this_frame)  - window_size
   if this_frame(i) > 8
        larger_cnt = 0;
        j = 0;
        sum_41 = 0;
        sum_p = 0;
        
        for j = -window_size : window_size
            sum_41 = sum_41 + this_frame(i + j);
            sum_p = sum_p + this_frame(i + j) * (i + j - 1);
            if (j == 0)
               continue; 
            end
            if this_frame(i) > this_frame(i+j)
                larger_cnt = larger_cnt + 1;
            end
        end
        if larger_cnt >= window_size * 2
            sum_p = sum_p / sum_41;
            maxid = sum_p;
            hold on;
            plot([i - j : i + j],this_frame(i-j:i+j), 'r');
            plot(maxid + 1,this_frame(i), 'g*');
            freq = scale_out * (maxid);
            %if (length(valid_pt) < 3)
            valid_pt = [valid_pt, freq];
            %end
        end
   end
end
for i = 1:length(valid_pt)
    devide_scale = round(valid_pt(i)/valid_pt(1));
    valid_pt(i) = valid_pt(i) / devide_scale;
end

fre = mean(valid_pt);
plot(fre / scale_out,50, 'b*');
end