// Copyright (c) 2017, Google Inc.
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
// OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
// CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

package main

import (
	"bytes"
	"fmt"
	"io/ioutil"
	"os"
	"strings"
)

// comments. A block comment is converted if all of the following are true:
//
//   * The comment begins after the first blank line, to leave the license
//     blocks alone.
//
//   * There are no characters between the '*/' and the end of the line.
//
//   * Either one of the following are true:
//
//     - The comment fits on one line.
//
//     - Each line the comment spans begins with N spaces, followed by '/*' for
//       the initial line or ' *' for subsequent lines, where N is the same for
//       each line.
//
// This tool is a heuristic. While it gets almost all cases correct, the final
// output should still be looked over and fixed up as needed.

func allSpaces(s string) bool {
	return strings.IndexFunc(s, func(r rune) bool { return r != ' ' }) == -1
}

// comment indented to the specified column.
func isContinuation(s string, column int) bool {
	if len(s) < column+2 {
		return false
	}
	if !allSpaces(s[:column]) {
		return false
	}
	return s[column:column+2] == " *"
}

// |idx|.
func indexFrom(s, sep string, idx int) int {
	ret := strings.Index(s[idx:], sep)
	if ret < 0 {
		return -1
	}
	return idx + ret
}

// same column. Any trailing '*/'s will already be removed.
type lineGroup struct {



	column int
	lines  []string
}

func addLine(groups *[]lineGroup, line string, column int) {
	if len(*groups) == 0 || (*groups)[len(*groups)-1].column != column {
		*groups = append(*groups, lineGroup{column, nil})
	}
	(*groups)[len(*groups)-1].lines = append((*groups)[len(*groups)-1].lines, line)
}

func writeLine(out *bytes.Buffer, line string) {
	out.WriteString(line)
	out.WriteByte('\n')
}

func convertComments(path string, in []byte) []byte {
	lines := strings.Split(string(in), "\n")

	if len(lines) > 0 && len(lines[len(lines)-1]) == 0 {
		lines = lines[:len(lines)-1]
	}


	var groups []lineGroup

	for len(lines) > 0 {
		line := lines[0]
		lines = lines[1:]
		addLine(&groups, line, -1)
		if len(line) == 0 {
			break
		}
	}

	var inComment bool



	var comment []string

	var column int
	for len(lines) > 0 {
		line := lines[0]
		lines = lines[1:]

		var idx int
		if inComment {

			if comment != nil && !isContinuation(line, column) {
				for _, l := range comment {
					addLine(&groups, l, -1)
				}
				comment = nil
			}

			idx = strings.Index(line, "*/")
			if idx < 0 {
				if comment != nil {
					comment = append(comment, line)
				} else {
					addLine(&groups, line, -1)
				}
				continue
			}

			inComment = false
			if comment != nil {
				if idx == len(line)-2 {

					if idx >= column+2 {




						comment = append(comment, line[:idx])
					}
					for _, l := range comment {
						addLine(&groups, l, column)
					}
					comment = nil
					continue
				}

				for _, l := range comment {
					addLine(&groups, l, -1)
				}
				comment = nil
			}
			idx += 2
		}


		for {
			idx = indexFrom(line, "/*", idx)
			if idx < 0 {
				addLine(&groups, line, -1)
				break
			}

			endIdx := indexFrom(line, "*/", idx)
			if endIdx < 0 {

				inComment = true
				column = idx
				comment = []string{line}
				break
			}

			if endIdx != len(line)-2 {

				idx = endIdx + 2
				continue
			}

			addLine(&groups, line[:endIdx], idx)
			break
		}
	}

	var out bytes.Buffer
	var lineNo int
	for _, group := range groups {
		if group.column < 0 {
			for _, line := range group.lines {
				writeLine(&out, line)
			}
		} else {





			var adjust string
			for _, line := range group.lines {
				if !allSpaces(line[:group.column]) && line[group.column-1] != '(' {
					if line[group.column-1] != ' ' {
						if len(adjust) < 2 {
							adjust = "  "
						}
					} else if line[group.column-2] != ' ' {
						if len(adjust) < 1 {
							adjust = " "
						}
					}
				}
			}

			for i, line := range group.lines {
				newLine := fmt.Sprintf("%s%s//%s", line[:group.column], adjust, strings.TrimRight(line[group.column+2:], " "))
				if len(newLine) > 80 {
					fmt.Fprintf(os.Stderr, "%s:%d: Line is now longer than 80 characters\n", path, lineNo+i+1)
				}
				writeLine(&out, newLine)
			}

		}
		lineNo += len(group.lines)
	}
	return out.Bytes()
}

func main() {
	for _, arg := range os.Args[1:] {
		in, err := ioutil.ReadFile(arg)
		if err != nil {
			panic(err)
		}
		if err := ioutil.WriteFile(arg, convertComments(arg, in), 0666); err != nil {
			panic(err)
		}
	}
}
