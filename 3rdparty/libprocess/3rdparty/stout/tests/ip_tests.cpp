#include <stdio.h>

#include <set>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <stout/foreach.hpp>
#include <stout/gtest.hpp>
#include <stout/ip.hpp>
#include <stout/net.hpp>
#include <stout/numify.hpp>
#include <stout/stringify.hpp>
#include <stout/strings.hpp>

using std::set;
using std::string;
using std::vector;


TEST(NetTest, LinkDevice)
{
  Try<set<string> > links = net::links();
  ASSERT_SOME(links);

  foreach(const string& link, links.get()) {
    Result<net::IPNetwork> network =
      net::IPNetwork::fromLinkDevice(link, AF_INET);

    EXPECT_FALSE(network.isError());

    if (network.isSome()) {
      string addr = stringify(network.get());
      string prefix = addr.substr(addr.find("/") + 1);
      ASSERT_SOME(numify<size_t>(prefix));
      EXPECT_EQ(network.get().prefix(), numify<size_t>(prefix).get());

      vector<string> tokens =
        strings::split(addr.substr(0, addr.find("/")), ".");

      EXPECT_EQ(4u, tokens.size());

      foreach (const string& token, tokens) {
        ASSERT_SOME(numify<int>(token));
        EXPECT_LE(0, numify<int>(token).get());
        EXPECT_GE(255, numify<int>(token).get());
      }
    }
  }

  EXPECT_ERROR(net::IPNetwork::fromLinkDevice("non-exist", AF_INET));
}


TEST(NetTest, ConstructIPv4)
{
  EXPECT_SOME(net::IP::parse("127.0.0.1", AF_INET));
  EXPECT_SOME(net::IP::parse("1.2.3.4", AF_INET));

  EXPECT_ERROR(net::IP::parse("123.1.1..2", AF_INET));
  EXPECT_ERROR(net::IP::parse("121.2.3.5.", AF_INET));
  EXPECT_ERROR(net::IP::parse("12.32.3.a", AF_INET));
  EXPECT_ERROR(net::IP::parse("hello world", AF_INET));
}


TEST(NetTest, ConstructIPv4Network)
{
  EXPECT_SOME(net::IPNetwork::parse("10.135.0.1/8", AF_INET));
  EXPECT_SOME(net::IPNetwork::parse("192.168.1.1/16", AF_INET));
  EXPECT_SOME(net::IPNetwork::parse("172.39.13.123/31", AF_INET));

  EXPECT_ERROR(net::IPNetwork::parse("123.1.1.2//23", AF_INET));
  EXPECT_ERROR(net::IPNetwork::parse("121.2.3.5/23/", AF_INET));
  EXPECT_ERROR(net::IPNetwork::parse("12.32.3.a/16", AF_INET));
  EXPECT_ERROR(net::IPNetwork::parse("hello moto/8", AF_INET));

  EXPECT_SOME(net::IPNetwork::create(net::IP(0x12345678), net::IP(0xffff0000)));
  EXPECT_SOME(net::IPNetwork::create(net::IP(0x12345678), net::IP(0xf0000000)));
  EXPECT_SOME(net::IPNetwork::create(net::IP(0x12345678), net::IP(0xffffffff)));
  EXPECT_SOME(net::IPNetwork::create(net::IP(0x12345678), net::IP(0)));

  EXPECT_ERROR(net::IPNetwork::create(net::IP(0x87654321), net::IP(0xff)));
  EXPECT_ERROR(net::IPNetwork::create(net::IP(0x87654321), net::IP(0xff00ff)));

  EXPECT_SOME(net::IPNetwork::create(net::IP(0x12345678), 16));
  EXPECT_SOME(net::IPNetwork::create(net::IP(0x12345678), 32));
  EXPECT_SOME(net::IPNetwork::create(net::IP(0x12345678), 0));

  EXPECT_ERROR(net::IPNetwork::create(net::IP(0x12345678), 123));

  uint32_t address = 0x01020304;
  uint32_t netmask = 0xff000000;

  Try<net::IPNetwork> network1 =
    net::IPNetwork::create(net::IP(address), net::IP(netmask));

  ASSERT_SOME(network1);
  EXPECT_EQ(net::IP(address), network1.get().address());
  EXPECT_EQ(net::IP(netmask), network1.get().netmask());
  EXPECT_EQ("1.2.3.4/8", stringify(network1.get()));

  Try<net::IPNetwork> network2 =
      net::IPNetwork::parse(stringify(network1.get()), AF_INET);

  ASSERT_SOME(network2);
  EXPECT_EQ(network1.get(), network2.get());
}
