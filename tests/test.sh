#!/bin/bash
# needs nc and xxd. frames are built by hand from SPEC.md, no client involved
PORT=9101
cd "$(dirname "$0")/.."
./bserve ./www $PORT > server.log 2>&1 &
SERVER=$!
sleep 1
trap "kill $SERVER 2>/dev/null" EXIT

pass=0
fail=0
check() {
    if [ "$2" = "0" ]; then echo "PASS  $1"; pass=$((pass+1)); else echo "FAIL  $1"; fail=$((fail+1)); fi
}

hexof() { printf '%s' "$1" | xxd -p | tr -d '\n'; }
h16() { printf '%04x' "$1"; }
filehex() { xxd -p "$1" | tr -d '\n'; }

# $1 = method byte (hex), $2 = path
rawreq() {
    local payload="$1$(h16 ${#2})$(hexof "$2")00"
    printf '42010100%08x%s' $(( ${#payload} / 2 )) "$payload"
}
send() { echo -n "$1" | xxd -r -p | nc -w 1 localhost $PORT | xxd -p | tr -d '\n'; }
statuses() { echo "$1" | grep -oE '42010200[0-9a-f]{8}[0-9a-f]{4}' | sed -E 's/.*(....)$/\1/' | tr '\n' ' '; }

out=$(send "$(rawreq 01 /index.html)")
[ "$(statuses "$out")" = "00c8 " ]; check "GET /index.html -> 200" $?
echo "$out" | grep -q "$(filehex www/index.html)"; check "body bytes are the file" $?
echo "$out" | grep -q '^42010200'; check "RESPONSE frame comes first" $?
echo "$out" | grep -q '42010301000000b0'; check "BODY frame has LAST flag and length 176" $?

out=$(send "$(rawreq 01 /)")
echo "$out" | grep -q "$(filehex www/index.html)"; check "/ gives index.html" $?

out=$(send "$(rawreq 01 /sub/page.txt)")
echo "$out" | grep -q "$(filehex www/sub/page.txt)"; check "file in sub folder" $?

out=$(send "$(rawreq 01 /big.bin)")
[[ "$out" == *"42010301$(printf '%08x' 100000)$(filehex www/big.bin)"* ]]; check "100000 byte file in one BODY frame" $?

out=$(send "$(rawreq 01 /empty.txt)")
[ "$(statuses "$out")" = "00c8 " ] && echo "$out" | grep -q '4201030100000000$'; check "empty file: empty BODY frame with LAST" $?

out=$(send "$(rawreq 01 /nope)")
[ "$(statuses "$out")" = "0194 " ]; check "missing file -> 404" $?

out=$(send "$(rawreq 01 /../secret)")
[ "$(statuses "$out")" = "0193 " ]; check "dot dot path -> 403" $?

out=$(send "$(rawreq 02 /index.html)")
[ "$(statuses "$out")" = "0190 " ]; check "unknown method -> 400" $?

out=$(send "$(rawreq 01 index.html)")
[ "$(statuses "$out")" = "0190 " ]; check "path without leading slash -> 400" $?

out=$(send "$(rawreq 01 /index.html)$(rawreq 01 /nope)$(rawreq 01 /about.html)")
[ "$(statuses "$out")" = "00c8 0194 00c8 " ]; check "3 requests on one connection" $?

bad="42010100""00000001""01"
out=$(send "$bad$(rawreq 01 /index.html)")
[ "$(statuses "$out")" = "0190 00c8 " ]; check "truncated payload -> 400, connection stays open" $?

unk="42017f00""00000003""aabbcc"
out=$(send "$unk$(rawreq 01 /index.html)")
[ "$(statuses "$out")" = "00c8 " ]; check "unknown frame type skipped, request still served" $?

out=$(send "58010100""00000000""$(rawreq 01 /index.html)")
[ "$(statuses "$out")" = "0190 " ]; check "bad magic -> 400 then close" $?

out=$(send "42020100""00000000""$(rawreq 01 /index.html)")
[ "$(statuses "$out")" = "0190 " ]; check "bad version -> 400 then close" $?

out=$(send "42010100""7fffffff")
[ "$(statuses "$out")" = "0190 " ]; check "huge length -> 400 then close" $?

echo "$pass passed, $fail failed"
rm -f server.log
[ $fail -eq 0 ]
