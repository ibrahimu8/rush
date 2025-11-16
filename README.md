# rush

fetch multiple URLs at the same time. thats it.

## what it does

curl fetches URLs one by one. if you need to hit 10 APIs, you wait for each one.

rush hits them all at once. 10 URLs that would take 20 seconds with curl? takes 2 seconds with rush.

## install

git clone https://github.com/ibrahimu8/rush
cd rush
make

## use it

one URL:
rush https://example.com

multiple URLs (this is where it shines):
rush https://api1.com https://api2.com https://api3.com

check if a bunch of services are up:
rush https://google.com https://github.com https://reddit.com

## why i built this

was tired of writing curl loops or dealing with xargs. needed something simple that just does parallel requests without all the complexity.

also built this entirely on my phone in termux which was interesting.

## whats it good for

- health checking multiple endpoints
- downloading several files quickly
- testing API response times
- any time you need to hit more than one URL and dont want to wait

## requirements

just libcurl. if curl works on your system, this will too.

## bugs probably exist

built and tested this in like a day. if something breaks open an issue.

## license

MIT do whatever
