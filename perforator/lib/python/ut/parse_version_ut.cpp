#include <gtest/gtest.h>
#include <perforator/lib/python/python.h>

#include <contrib/libs/re2/re2/re2.h>
#include <contrib/libs/re2/re2/stringpiece.h>

using namespace NPerforator::NLinguist::NPython;

// Declare the TryScanVersion function from python.cpp
namespace NPerforator::NLinguist::NPython {
    TMaybe<TPythonVersion> TryScanVersion(TConstArrayRef<char> data);
}

TEST(PythonVersionParsing, TryScanVersion) {
    struct TTestCase {
        TString VersionString;
        bool ShouldParse;
        TPythonVersion ExpectedVersion;
    };

    TVector<TTestCase> testCases = {
        {"Python 3.10.0", true, {3, 10, 0}},
        {"Python 3.10.1", true, {3, 10, 1}},

        {"Python 2.7", true, {2, 7, 0}},
        {"Python 2.7.10", true, {2, 7, 10}},
        {"Python 3.8.15", true, {3, 8, 15}},
        {"Python 3.12.0", true, {3, 12, 0}},
        {"Python 3.12.1", true, {3, 12, 1}},
        {"Python 2.8.3", true, {2, 8, 3}},

        {"Python version 3.10.0, 64 bit", true, {3, 10, 0}},
        {"Version: Python 2.7.15", true, {2, 7, 15}},

        {"Python 4.0.0", false, {}},  // Major not 2 or 3
        {"No version here", false, {}},
        {"Python v9.8.7", false, {}},
    };

    for (const auto& testCase : testCases) {
        TMaybe<TPythonVersion> result = TryScanVersion(TConstArrayRef<char>{testCase.VersionString.data(), testCase.VersionString.size()});

        if (testCase.ShouldParse) {
            ASSERT_TRUE(result.Defined()) << "Failed to parse version from string: '" << testCase.VersionString << "'";

            TString expectedVersionStr =
                std::to_string(testCase.ExpectedVersion.MajorVersion) + "." +
                std::to_string(testCase.ExpectedVersion.MinorVersion) + "." +
                std::to_string(testCase.ExpectedVersion.MicroVersion);

            TString actualVersionStr =
                std::to_string(result->MajorVersion) + "." +
                std::to_string(result->MinorVersion) + "." +
                std::to_string(result->MicroVersion);

            EXPECT_EQ(result->MajorVersion, testCase.ExpectedVersion.MajorVersion)
                << "Major version mismatch for input: '" << testCase.VersionString
                << "', expected: " << expectedVersionStr
                << ", got: " << actualVersionStr;

            EXPECT_EQ(result->MinorVersion, testCase.ExpectedVersion.MinorVersion)
                << "Minor version mismatch for input: '" << testCase.VersionString
                << "', expected: " << expectedVersionStr
                << ", got: " << actualVersionStr;

            EXPECT_EQ(result->MicroVersion, testCase.ExpectedVersion.MicroVersion)
                << "Micro version mismatch for input: '" << testCase.VersionString
                << "', expected: " << expectedVersionStr
                << ", got: " << actualVersionStr;
        } else {
            EXPECT_FALSE(result.Defined()) << "Should not parse version from string: '" << testCase.VersionString << "'";
        }
    }
}
