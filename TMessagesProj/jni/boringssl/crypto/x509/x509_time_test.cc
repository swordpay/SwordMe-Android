/*
 * Copyright 2017 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */


#include <openssl/x509.h>

#include <string.h>
#include <time.h>

#include <gtest/gtest.h>
#include <openssl/asn1.h>

struct TestData {
  const char *data;
  int type;
  time_t cmp_time;

  int expected;
};

static TestData kX509CmpTests[] = {
    {
        "20170217180154Z",
        V_ASN1_GENERALIZEDTIME,

        1487354514,
        -1,
    },
    {
        "20170217180154Z",
        V_ASN1_GENERALIZEDTIME,

        1487354515,
        -1,
    },
    {
        "20170217180154Z",
        V_ASN1_GENERALIZEDTIME,

        1487354513,
        1,
    },

    {
        "170217180154Z",
        V_ASN1_UTCTIME,

        1487354514,
        -1,
    },
    {
        "170217180154Z",
        V_ASN1_UTCTIME,

        1487354515,
        -1,
    },
    {
        "170217180154Z",
        V_ASN1_UTCTIME,

        1487354513,
        1,
    },

    {
        "990217180154Z",
        V_ASN1_UTCTIME,

        919274514,
        -1,
    },
    {
        "990217180154Z",
        V_ASN1_UTCTIME,

        919274515,
        -1,
    },
    {
        "990217180154Z",
        V_ASN1_UTCTIME,

        919274513,
        1,
    },

    {

        "20170217180154",
        V_ASN1_GENERALIZEDTIME,
        0,
        0,
    },
    {

        "170217180154",
        V_ASN1_UTCTIME,
        0,
        0,
    },
    {

        "201702171801Z",
        V_ASN1_GENERALIZEDTIME,
        0,
        0,
    },
    {

        "1702171801Z",
        V_ASN1_UTCTIME,
        0,
        0,
    },
    {

        "20170217180154.001Z",
        V_ASN1_GENERALIZEDTIME,
        0,
        0,
    },
    {

        "170217180154.001Z",
        V_ASN1_UTCTIME,
        0,
        0,
    },
    {

        "20170217180154+0100",
        V_ASN1_GENERALIZEDTIME,
        0,
        0,
    },
    {

        "170217180154+0100",
        V_ASN1_UTCTIME,
        0,
        0,
    },
    {

        "2017021718015400Z",
        V_ASN1_GENERALIZEDTIME,
        0,
        0,
    },
    {

        "17021718015400Z",
        V_ASN1_UTCTIME,
        0,
        0,
    },
    {

        "2017021718015aZ",
        V_ASN1_GENERALIZEDTIME,
        0,
        0,
    },
    {

        "17021718015aZ",
        V_ASN1_UTCTIME,
        0,
        0,
    },
    {

        "20170217180154Zlongtrailinggarbage",
        V_ASN1_GENERALIZEDTIME,
        0,
        0,
    },
    {

        "170217180154Zlongtrailinggarbage",
        V_ASN1_UTCTIME,
        0,
        0,
    },
    {

        "20170217180154Z",
        V_ASN1_UTCTIME,
        0,
        0,
    },
    {

        "170217180154Z",
        V_ASN1_GENERALIZEDTIME,
        0,
        0,
    },
    {

        "20170217180154Z",
        V_ASN1_OCTET_STRING,
        0,
        0,
    },
};

TEST(X509TimeTest, TestCmpTime) {
  for (auto &test : kX509CmpTests) {
    SCOPED_TRACE(test.data);

    ASN1_TIME t;

    memset(&t, 0, sizeof(t));
    t.type = test.type;
    t.data = (unsigned char*) test.data;
    t.length = strlen(test.data);

    EXPECT_EQ(test.expected,
              X509_cmp_time(&t, &test.cmp_time));
  }
}

TEST(X509TimeTest, TestCmpTimeCurrent) {
  time_t now = time(NULL);

  bssl::UniquePtr<ASN1_TIME> asn1_before(ASN1_TIME_adj(NULL, now, -1, 0));
  bssl::UniquePtr<ASN1_TIME> asn1_after(ASN1_TIME_adj(NULL, now, 1, 0));

  ASSERT_EQ(-1, X509_cmp_time(asn1_before.get(), NULL));
  ASSERT_EQ(1, X509_cmp_time(asn1_after.get(), NULL));
}
