# function assumes 1 event per file, ie. one drop, one crash, etc etc
import statistics

with open ("configData_floordrop.txt", "r") as file:
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
            mg = int(i[1][:-5])
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
idx_range = 30
start_time = None
endtime = None

if max_mg != 0:
    if max_mg_idx < idx_range:
        start_idx = 0
        end_idx = max_mg_idx + idx_range
    elif len(mg_values) - max_mg_idx < idx_range:
        start_idx = max_mg_idx - idx_range
        end_idx = len(mg_values) - 1
    else:
        start_idx = max_mg_idx - idx_range
        end_idx = max_mg_idx + idx_range

    for x in range(start_idx, end_idx + 1):
        if in_crash_range == 0 and mg_values[x] > avg_mg + std_dev:
            in_crash_range = 1
            start_time = time_values[x]
        elif in_crash_range == 1 and mg_values[x] > avg_mg + std_dev:
            continue
        elif in_crash_range == 1 and mg_values[x] < avg_mg + std_dev:
            in_crash_range = 0
            end_time = time_values[x]

if start_time == None or end_time == None:
    raise ValueError("")
impact_time = end_time - start_time #in ms
line = [f"\n{crash}, {max_mg}, {min_mg}, {avg_mg}, {impact_time}"]
# if crash == 0:
#     with open("non-crashes.csv", "a") as no_crash_file:
        
# elif crash == 1:
#     with open("crashes.csv", "a") as crash_file:



print(max_mg)
print(min_mg)
print(avg_mg)
        

