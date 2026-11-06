# function assumes 1 event per file, ie. one drop, one crash, etc etc
import statistics

with open ("capture.txt", "r") as file:
    crash = int(input("Was this a crash? Reply 1 or 0: "))
    mg_values = []
    time_values = []
    max_mg = 0
    min_mg = 28000
    avg_mg = 0
    max_mg_idx = 0
    std_dev = 0
    lines_skipped = 0

    max_mg_time = []

    lines = file.readlines()
    for c, i in enumerate(lines):
        i = i.split(" ")
        if len(i) < 3:
            lines_skipped += 1
            continue
        else:
            mg = int(i[1][:-4])
            time = int(i[2][:-5])
            mg_values.append(mg)
            time_values.append(time)

            avg_mg += mg

            if mg > max_mg:
                max_mg = mg
                max_mg_idx = c - lines_skipped

            if mg < min_mg:
                min_mg = mg
                
    avg_mg /= len(mg_values)

std_dev = statistics.pstdev(mg_values)

in_crash_range = 0
start_idx = max_mg_idx
end_idx = max_mg_idx
start_time = -1
end_time = -1
start_value = -1
end_value = -1
threshold = avg_mg + (2*std_dev)

if max_mg != 0:
    while(mg_values[start_idx - 1] > threshold): #Left Check for time and spike to plot
        if start_idx <= 1:
            start_idx = 0
            break
        start_idx -= 1
    start_value = mg_values[start_idx]
    start_time = time_values[start_idx]

    while(mg_values[end_idx + 1] > threshold): #Right check for the timeand spike to plot
        if end_idx >= len(mg_values) - 2:
            end_idx = len(mg_values) - 1
            break
        end_idx += 1
    end_value = mg_values[end_idx]
    end_time = time_values[end_idx]



if start_time == -1 or end_time == -1:
    raise ValueError("Time Not Defined, Incident not Detected")
impact_time = end_time - start_time #in ms
line = f"\n{crash}, {max_mg}, {min_mg}, {avg_mg}, {impact_time}"
print(line)
# if crash == 0:
#     with open("non-crashes.csv", "a") as no_crash_file:
        
# elif crash == 1:
#     with open("crashes.csv", "a") as crash_file:



# print(max_mg)
# print(min_mg)
# print(avg_mg)
        

