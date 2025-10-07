package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"os"
	"strconv"
	"strings"
)

// getRSSKb gets RSS (Resident Set Size) in kilobytes from /proc/self/status
func getRSSKb() int64 {
	file, err := os.Open("/proc/self/status")
	if err != nil {
		return -1
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		if strings.HasPrefix(line, "VmRSS:") {
			// Extract the number from "VmRSS:    12345 kB"
			fields := strings.Fields(line)
			if len(fields) >= 2 {
				rss, err := strconv.ParseInt(fields[1], 10, 64)
				if err == nil {
					return rss
				}
			}
		}
	}
	return -1
}

func main() {
	if len(os.Args) != 2 {
		fmt.Fprintf(os.Stderr, "Usage: %s <input_file>\n", os.Args[0])
		os.Exit(1)
	}

	// Measure RSS before loading
	rssBefore := getRSSKb()

	file, err := os.Open(os.Args[1])
	if err != nil {
		fmt.Fprintf(os.Stderr, "Failed to open the file: %v\n", err)
		os.Exit(1)
	}
	defer file.Close()

	var jsonArray []map[string]interface{}
	scanner := bufio.NewScanner(file)

	for scanner.Scan() {
		line := scanner.Text()
		var obj map[string]interface{}
		if err := json.Unmarshal([]byte(line), &obj); err != nil {
			fmt.Fprintf(os.Stderr, "Failed to parse JSON: %v\n", err)
			continue
		}
		jsonArray = append(jsonArray, obj)
	}

	if err := scanner.Err(); err != nil {
		fmt.Fprintf(os.Stderr, "Error reading file: %v\n", err)
		os.Exit(1)
	}

	// Measure RSS after loading
	rssAfter := getRSSKb()

	if rssBefore >= 0 && rssAfter >= 0 {
		rssDiff := rssAfter - rssBefore
		fmt.Printf("golang RSS: %d KB\n", rssDiff)
	} else {
		fmt.Fprintln(os.Stderr, "RSS measurement not available on this platform")
	}
}
