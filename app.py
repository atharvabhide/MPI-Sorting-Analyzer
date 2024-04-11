import streamlit as st
import subprocess
import pandas as pd
import re

def run_c_script(input_size, script_name):
    if input_size > 32000:
        st.warning("Input size is too large. Consider reducing it to 32000 or less.")
        return None
    command = ["mpiexec", "-n", "4", script_name, str(input_size)]
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()
    script_name = script_name.split('/')[-1].split('.')[0]
    if process.returncode == 0:
        st.success(f"Successfully executed {script_name}.")
        return stdout.decode('utf-8')
    else:
        st.error(f"Error executing {script_name}.")
        st.text(stderr.decode('utf-8'))
        return None

def parse_output(output, algo_name):
    lines = output.strip().split('\n')
    data = []
    for line in lines[1:]:
        parts = re.split(r'\s+', line.strip())
        parts.insert(0, algo_name)
        data.append(parts)
    return data

def main():
    st.title("MPI Sorting Algorithm Analyzer")

    input_size = st.number_input("Enter the input size:", min_value=1, step=1)
    
    if st.button("Execute"):
        st.write("Executing all sorting algorithms for input size:", input_size)
        algorithms = [
            {"name": "Quick Sort", "script": "bin/quicksort.exe"},
            {"name": "Merge Sort", "script": "bin/mergesort.exe"},
            {"name": "Bubble Sort", "script": "bin/bubblesort.exe"},
            {"name": "Selection Sort", "script": "bin/selectionsort.exe"},
            {"name": "Insertion Sort", "script": "bin/insertionsort.exe"}
        ]
        all_data = []
        for algo in algorithms:
            output = run_c_script(input_size, algo["script"])
            if output:
                data = parse_output(output, algo["name"])
                all_data.extend(data)
        
        if all_data:
            df = pd.DataFrame(all_data, columns=["Algorithm", "Array Size", "Time without MPI (s)", "Time with MPI (s)"])
            st.markdown("<h2 style='text-align: center;'>Comparison Chart</h2>", unsafe_allow_html=True)
            st.table(df.style.set_properties(**{'text-align': 'center'}).hide_index())

if __name__ == "__main__":
    main()
