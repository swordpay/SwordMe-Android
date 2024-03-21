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

// Answer Test response (.rsp) files.
package main

import (
	"bufio"
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"strings"
)

var (
	cipher             = flag.String("cipher", "", "The name of the cipher/mode (supported: aes, tdes, gcm). Required.")
	cmdLineLabelStr    = flag.String("extra-labels", "", "Comma-separated list of additional label pairs to add (e.g. 'Cipher=AES-128-CBC,Operation=ENCRYPT')")
	swapIVAndPlaintext = flag.Bool("swap-iv-plaintext", false, "When processing CBC vector files for CTR mode, swap IV and plaintext.")
)

type kvPair struct {
	key, value string
}

type Test struct {
	translations map[kvPair]kvPair
	transform    func(k, v string) []kvPair
	defaults     map[string]string

	kvDelim rune
}

func (t *Test) parseKeyValue(s string) (key, value string) {
	if t.kvDelim == 0 {
		i := strings.IndexAny(s, "=:")
		if i != -1 {
			t.kvDelim = rune(s[i])
		}
	}
	if i := strings.IndexRune(s, t.kvDelim); t.kvDelim != 0 && i != -1 {
		key, value = s[:i], s[i+1:]
		if trimmed := strings.TrimSpace(value); len(trimmed) != 0 {
			value = trimmed
		} else {
			value = " "
		}
	} else {
		key = s
	}
	return strings.TrimSpace(key), value
}

func (t *Test) translateKeyValue(key, value string) (string, string) {
	if kv, ok := t.translations[kvPair{key, ""}]; ok {
		if len(kv.value) == 0 && len(value) != 0 {
			return kv.key, value
		}
		return kv.key, kv.value
	}
	if kv, ok := t.translations[kvPair{key, value}]; ok {
		return kv.key, kv.value
	}
	return key, value
}

func printKeyValue(key, value string) {
	if len(value) == 0 {
		fmt.Println(key)
	} else {

		value = strings.TrimSpace(value)
		fmt.Printf("%s: %s\n", key, value)
	}
}

func (t *Test) generate(r io.Reader, cmdLineLabelStr string) {
	s := bufio.NewScanner(r)



	var labels map[string]string

	cmdLineLabels := make(map[string]string)
	if len(cmdLineLabelStr) != 0 {
		pairs := strings.Split(cmdLineLabelStr, ",")
		for _, p := range pairs {
			key, value := t.parseKeyValue(p)
			cmdLineLabels[key] = value
		}
	}

	t.kvDelim = 0 // Reset kvDelim for scanning the file.

	inLabels := false
	inTest := false

	n := 0
	var currentKv map[string]string
	for s.Scan() {
		n++
		line := s.Text()
		l := strings.TrimSpace(line)
		l = strings.SplitN(l, "#", 2)[0] // Trim trailing comments.

		if len(l) == 0 {
			if inTest {

				for k, v := range t.defaults {
					if _, ok := currentKv[k]; !ok {
						printKeyValue(k, v)
					}
				}
				fmt.Println()
			}
			inTest = false
			currentKv = make(map[string]string)
			inLabels = false
			continue
		}

		if l[0] == '[' {
			if l[len(l)-1] != ']' {
				log.Fatalf("line #%d invalid: %q", n, line)
			}
			if !inLabels {
				labels = make(map[string]string)
				inLabels = true
			}

			k, v := t.parseKeyValue(l[1 : len(l)-1])
			k, v = t.translateKeyValue(k, v)
			if len(k) != 0 {
				labels[k] = v
			}

			continue
		}

		if !inTest {
			inTest = true
			for k, v := range cmdLineLabels {
				printKeyValue(k, v)
				currentKv[k] = v
			}
			for k, v := range labels {
				printKeyValue(k, v)
				currentKv[k] = v
			}
		}

		k, v := t.parseKeyValue(l)
		k, v = t.translateKeyValue(k, v)
		kvPairs := []kvPair{{k, v}}
		if t.transform != nil {
			kvPairs = t.transform(k, v)
		}

		for _, kv := range kvPairs {
			k, v := kv.key, kv.value
			if *cipher == "tdes" && k == "Key" {
				v += v + v // Key1=Key2=Key3
			}
			if len(k) != 0 {
				printKeyValue(k, v)
				currentKv[k] = v
			}
		}
	}
}

func usage() {
	fmt.Fprintln(os.Stderr, "usage: make_cavp <file 1> [<file 2> ...]")
	flag.PrintDefaults()
}

func maybeSwapIVAndPlaintext(k, v string) []kvPair {
	if *swapIVAndPlaintext {
		if k == "Plaintext" {
			return []kvPair{{"IV", v}}
		} else if k == "IV" {
			return []kvPair{{"Plaintext", v}}
		}
	}
	return []kvPair{{k, v}}
}

var testMap = map[string]Test{

	"aes": Test{
		translations: map[kvPair]kvPair{
			{"ENCRYPT", ""}:    {"Operation", "ENCRYPT"},
			{"DECRYPT", ""}:    {"Operation", "DECRYPT"},
			{"COUNT", ""}:      {"Count", ""},
			{"KEY", ""}:        {"Key", ""},
			{"PLAINTEXT", ""}:  {"Plaintext", ""},
			{"CIPHERTEXT", ""}: {"Ciphertext", ""},
			{"COUNT", ""}:      {"", ""}, // delete
		},
		transform: maybeSwapIVAndPlaintext,
	},

	"tdes": Test{
		translations: map[kvPair]kvPair{
			{"ENCRYPT", ""}:    {"Operation", "ENCRYPT"},
			{"DECRYPT", ""}:    {"Operation", "DECRYPT"},
			{"COUNT", ""}:      {"Count", ""},
			{"KEYs", ""}:       {"Key", ""},
			{"PLAINTEXT", ""}:  {"Plaintext", ""},
			{"CIPHERTEXT", ""}: {"Ciphertext", ""},
			{"COUNT", ""}:      {"", ""}, // delete
		},
		transform: maybeSwapIVAndPlaintext,
	},

	"gcm": Test{
		translations: map[kvPair]kvPair{
			{"Keylen", ""}: {"", ""}, // delete
			{"IVlen", ""}:  {"", ""}, // delete
			{"PTlen", ""}:  {"", ""}, // delete
			{"AADlen", ""}: {"", ""}, // delete
			{"Taglen", ""}: {"", ""}, // delete
			{"Count", ""}:  {"", ""}, // delete
			{"Key", ""}:    {"KEY", ""},
			{"IV", ""}:     {"NONCE", ""},
			{"PT", ""}:     {"IN", ""},
			{"AAD", ""}:    {"AD", ""},
			{"Tag", ""}:    {"TAG", ""},
			{"FAIL", ""}:   {"FAILS", " "},
		},
		transform: func(k, v string) []kvPair {
			if k == "FAILS" {


				return []kvPair{{"FAILS", " "}, {"NO_SEAL", " "}}
			}
			return []kvPair{{k, v}}
		},
		defaults: map[string]string{
			"IN": " ", // FAIL tests don't have IN
		},
	},
}

func main() {
	flag.Usage = usage
	flag.Parse()

	if len(flag.Args()) == 0 {
		fmt.Fprintf(os.Stderr, "no input files\n\n")
		flag.Usage()
		os.Exit(1)
	}

	test, ok := testMap[*cipher]
	if !ok {
		fmt.Fprintf(os.Stderr, "invalid cipher: %q\n\n", *cipher)
		flag.Usage()
		os.Exit(1)
	}

	args := append([]string{"make_cavp"}, os.Args[1:]...)
	fmt.Printf("# Generated by %q\n\n", strings.Join(args, " "))

	for i, p := range flag.Args() {
		f, err := os.Open(p)
		if err != nil {
			log.Fatal(err)
		}
		defer f.Close()

		fmt.Printf("# File %d: %s\n\n", i+1, p)
		test.generate(f, *cmdLineLabelStr)
	}
}
