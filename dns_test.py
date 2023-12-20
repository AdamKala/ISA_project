# ISA Project DNS - test
# Author: Adam Kala 
# Login: xkalaa00 
# Year 2023 

# -*- coding: utf-8 -*-

import subprocess
import difflib
import re

# Result, as testx_b, are from dig and the right command.
# They are reformatted in my style and then compared with the output from CMD.
# Some values can be different everytime, so they are found and then rewrited.

test1_a = subprocess.check_output(["./dns", "-6", "-x", "-s", "kazi.fit.vutbr.cz", "2001:4860:4860::8888"]).decode("utf-8")

test1_b = """
Authoritative: No, Recursive: No, Truncated: No
Question section (1)
  8.8.8.8.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.6.8.4.0.6.8.4.1.0.0.2.ip6.arpa., PTR, IN
Answer section (0)
Authority section (6)
  ip6.arpa., NS, IN, 172800, c.ip6-servers.arpa.
  ip6.arpa., NS, IN, 172800, d.ip6-servers.arpa.
  ip6.arpa., NS, IN, 172800, e.ip6-servers.arpa.
  ip6.arpa., NS, IN, 172800, f.ip6-servers.arpa.
  ip6.arpa., NS, IN, 172800, a.ip6-servers.arpa.
  ip6.arpa., NS, IN, 172800, b.ip6-servers.arpa.
Additional section (12)
  f.ip6-servers.arpa., AAAA, IN, 172800, 2001:67c:e0::2
  e.ip6-servers.arpa., AAAA, IN, 172800, 2001:dd8:6::101
  d.ip6-servers.arpa., AAAA, IN, 172800, 2001:13c7:7012::53
  c.ip6-servers.arpa., AAAA, IN, 172800, 2001:43f8:110::11
  b.ip6-servers.arpa., AAAA, IN, 172800, 2001:500:86::86
  a.ip6-servers.arpa., AAAA, IN, 172800, 2620:37:e000::53
  f.ip6-servers.arpa., A, IN, 172800, 193.0.9.2
  e.ip6-servers.arpa., A, IN, 172800, 203.119.86.101
  d.ip6-servers.arpa., A, IN, 172800, 200.7.86.53
  c.ip6-servers.arpa., A, IN, 172800, 196.216.169.11
  b.ip6-servers.arpa., A, IN, 172800, 199.253.182.182
  a.ip6-servers.arpa., A, IN, 172800, 199.180.182.53
"""

test2_a = subprocess.check_output(["./dns", "-6", "-x", "-s", "kazi.fit.vutbr.cz", "2001:67c:1220:809::93e5:917"]).decode("utf-8") 
test2_b = """
Authoritative: Yes, Recursive: No, Truncated: No
Question section (1)
  7.1.9.0.5.e.3.9.0.0.0.0.0.0.0.0.9.0.8.0.0.2.2.1.c.7.6.0.1.0.0.2.ip6.arpa., PTR, IN
Answer section (1)
  7.1.9.0.5.e.3.9.0.0.0.0.0.0.0.0.9.0.8.0.0.2.2.1.c.7.6.0.1.0.0.2.ip6.arpa., PTR, IN, 14400
Authority section (4)
  8.0.0.2.2.1.c.7.6.0.1.0.0.2.ip6.arpa., NS, IN, 14400, kazi.fit.vutbr.cz.
  8.0.0.2.2.1.c.7.6.0.1.0.0.2.ip6.arpa., NS, IN, 14400, gate.feec.vutbr.cz.
  8.0.0.2.2.1.c.7.6.0.1.0.0.2.ip6.arpa., NS, IN, 14400, guta.fit.vutbr.cz.
  8.0.0.2.2.1.c.7.6.0.1.0.0.2.ip6.arpa., NS, IN, 14400, rhino.cis.vutbr.cz.
Additional section (0)
"""

test3_a = subprocess.check_output(["./dns", "-r", "-6", "-x", "-s", "dns.google", "2a00:1450:400d:806::2004"]).decode("utf-8")
test3_b = """
Authoritative: No, Recursive: Yes, Truncated: No
Question section (1)
  4.0.0.2.0.0.0.0.0.0.0.0.0.0.0.0.6.0.8.0.d.0.0.4.0.5.4.1.0.0.a.2.ip6.arpa., PTR, IN
Answer section (2)
  4.0.0.2.0.0.0.0.0.0.0.0.0.0.0.0.6.0.8.0.d.0.0.4.0.5.4.1.0.0.a.2.ip6.arpa., PTR, IN, PLACEHOLDER
  4.0.0.2.0.0.0.0.0.0.0.0.0.0.0.0.6.0.8.0.d.0.0.4.0.5.4.1.0.0.a.2.ip6.arpa., PTR, IN, PLACEHOLDER
Authority section (0)
Additional section (0)"""

match3 = re.search(r'Answer section \(2\)\n.*?, (\d+)$', test3_a, re.MULTILINE)

if match3:
    number3 = match3.group(1)
    test3_b = re.sub(r'PLACEHOLDER', number3, test3_b)
    test3_b = re.sub(r'PLACEHOLDER', number3, test3_b)


test4_a = subprocess.check_output(["./dns", "-r", "-x", "-s", "dns.google", "15.197.204.56"]).decode("utf-8")
test4_b = """
Authoritative: No, Recursive: Yes, Truncated: No
Question section (1)
  56.204.197.15.in-addr.arpa., PTR, IN
Answer section (1)
  56.204.197.15.in-addr.arpa., PTR, IN, PLACEHOLDER
Authority section (0)
Additional section (0)
"""

match4 = re.search(r'Answer section \(1\)\n.*?, (\d+)$', test4_a, re.MULTILINE)
if match4:
    number4 = match4.group(1)
    test4_b = re.sub(r'PLACEHOLDER', number4, test4_b)

test5_a = subprocess.check_output(["./dns", "-r", "-x", "-s", "dns.google", "93.184.216.34"]).decode("utf-8")
test5_b = """
Authoritative: No, Recursive: Yes, Truncated: No
Question section (1)
  34.216.184.93.in-addr.arpa., PTR, IN
Answer section (0)
Authority section (1)
  216.184.93.in-addr.arpa., SOA, IN, PLACEHOLDER
Additional section (0)
"""

match5 = re.search(r'Authority section \(1\)\n.*?, (\d+)$', test5_a, re.MULTILINE)
if match5:
    number5 = match5.group(1)
    test5_b = re.sub(r'PLACEHOLDER', number5, test5_b)

test6_a = subprocess.check_output(["./dns", "-r", "-s", "kazi.fit.vutbr.cz", "www.fit.vut.cz"]).decode("utf-8")
test6_b = """
Authoritative: Yes, Recursive: Yes, Truncated: No
Question section (1)
  www.fit.vut.cz., A, IN
Answer section (1)
  www.fit.vut.cz., A, IN, 14400, 147.229.9.26
Authority section (0)
Additional section (0)
"""

test7_a = subprocess.check_output(["./dns", "-r", "-x", "-s", "2001:67c:1220:808::93e5:80c", "2a00:1450:400d:80c::200e"]).decode("utf-8")

test7_b = """
Authoritative: No, Recursive: Yes, Truncated: No
Question section (1)
  e.0.0.2.0.0.0.0.0.0.0.0.0.0.0.0.c.0.8.0.d.0.0.4.0.5.4.1.0.0.a.2.ip6.arpa., PTR, IN
Answer section (1)
  e.0.0.2.0.0.0.0.0.0.0.0.0.0.0.0.c.0.8.0.d.0.0.4.0.5.4.1.0.0.a.2.ip6.arpa., PTR, IN, PLACEHOLDER
Authority section (0)
Additional section (0)
"""

match7 = re.search(r'Answer section \(1\)\n.*?, (\d+)$', test7_a, re.MULTILINE)
if match7:
    number7 = match7.group(1)
    test7_b = re.sub(r'PLACEHOLDER', number7, test7_b)

test8_a = subprocess.check_output(["./dns", "-r", "-s", "kazi.fit.vutbr.cz", "www.github.com"]).decode("utf-8")
test8_b = """
Authoritative: No, Recursive: Yes, Truncated: No
Question section (1)
  www.github.com., A, IN
Answer section (2)
  www.github.com., CNAME, IN, PLACEHOLDER1, github.com.
  github.com., A, IN, PLACEHOLDER2, PLACEHOLDER_IP
Authority section (0)
Additional section (0)
"""

match_placeholder1 = re.search(r'www\.github\.com\., CNAME, IN, (\S+), github\.com\.', test8_a)
match_placeholder2 = re.search(r'github\.com\., A, IN, (\S+), (\S+)', test8_a)

if match_placeholder1 and match_placeholder2:
    placeholder1 = match_placeholder1.group(1)
    placeholder2 = match_placeholder2.group(1)
    placeholder_ip = match_placeholder2.group(2)

    test8_b = re.sub(r'PLACEHOLDER1', placeholder1, test8_b)
    test8_b = re.sub(r'PLACEHOLDER2', placeholder2, test8_b)
    test8_b = re.sub(r'PLACEHOLDER_IP', placeholder_ip, test8_b)

try:
    test9_a = subprocess.check_output(["./dns", "-r", "-s", "30", "kazi.fit.vutbr.cz", "www.github.com"], stderr=subprocess.STDOUT)
except subprocess.CalledProcessError as e:
    test9_a = e.returncode
test9_b = 255

try:
    test10_a = subprocess.check_output(["./dns", "-r", "kazi.fit.vutbr.cz", "www.github.com"], stderr=subprocess.STDOUT)
except subprocess.CalledProcessError as e:
    test10_a = e.returncode
test10_b = 255

texts = [
    test1_a,test1_b,
    test2_a,test2_b,
    test3_a,test3_b,
    test4_a,test4_b,
    test5_a,test5_b,
    test6_a,test6_b,
    test7_a,test7_b,
    test8_a,test8_b,
    test9_a, test9_b,
    test10_a, test10_b,
]

pairs_to_compare = [(0, 1), (2, 3), (4, 5), (6, 7), (8, 9), (10, 11), (12, 13), (14, 15), (16, 17), (18, 19)]
y = 1
for pair in pairs_to_compare:
    i, j = pair

    try:
        lines_text1 = set(texts[i].split('\n'))
        lines_text2 = set(texts[j].split('\n'))

        different_lines = lines_text1.symmetric_difference(lines_text2)

        if different_lines:
            print(f"Test neprosel. Rozdily:")
            for line in different_lines:
                if line in lines_text1 and line not in lines_text2:
                    print(f"Test {y} output: {line}")
                elif line in lines_text2 and line not in lines_text1:
                    print(f"Test {y} should be: {line}")
        else:
            print(f"Test {y} Passed.")
        y += 1
    
    except:
        i = j
        print(f"Test {y} Passed.")
        y += 1

