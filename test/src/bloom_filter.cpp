#include <gtest/gtest.h>
#include <sample.hpp>
#include <kmer.hpp>
#include <bloom_filter.hpp>
#include <iostream>
#include <boost/filesystem/operations.hpp>
#include <helpers.hpp>
#include <file/sample_header.hpp>
#include <file/frequency_header.hpp>
#include <file/util.hpp>

namespace {
    using namespace genome;

    std::string in_dir_1 = "test/resources/bloom_filter/input_1/";
    std::string in_dir_2 = "test/resources/bloom_filter/input_2/";
    std::string in_dir_3 = "test/resources/bloom_filter/input_3/";
    std::string out_dir = "test/out/bloom_filter/";
    std::string out_file_1 = out_dir + "[sample_1-sample_3].g_blo";
    std::string out_file_2 = out_dir + "[sample_4-sample_4].g_blo";
    std::string out_file_3 = out_dir + "[sample_1-sample_9].g_blo";
    std::string out_file_4 = out_dir + "[sample_9-sample_9].g_blo";
    std::string sample_1 = in_dir_1 + "sample_1.g_sam";
    std::string sample_2 = in_dir_1 + "sample_2.g_sam";
    std::string sample_3 = in_dir_1 + "sample_3.g_sam";
    std::string sample_4 = in_dir_2 + "sample_4.g_sam";
    std::string sample_9 = in_dir_3 + "sample_9.g_sam";
    size_t bloom_filter_size = 200000;
    size_t block_size = 1;
    size_t num_hashes = 7;


    TEST(bloom_filter, contains) {
        boost::filesystem::remove_all(out_dir);
        genome::bloom_filter::process_all_in_directory(in_dir_1, out_dir, bloom_filter_size, block_size, num_hashes);

        bloom_filter bf;
        file::deserialize(out_file_1, bf);

        genome::sample<31> s;
        file::deserialize(sample_1, s);
        for (auto kmer: s.data()) {
            ASSERT_TRUE(bf.contains(kmer, 0));
        }

        file::deserialize(sample_2, s);
        for (auto kmer: s.data()) {
            ASSERT_TRUE(bf.contains(kmer, 1));
        }

        file::deserialize(sample_3, s);
        for (auto kmer: s.data()) {
            ASSERT_TRUE(bf.contains(kmer, 2));
        }
    }

    TEST(bloom_filter, contains_big_block) {
        boost::filesystem::remove_all(out_dir);
        genome::bloom_filter::process_all_in_directory(in_dir_3, out_dir, bloom_filter_size, 2, num_hashes);

        bloom_filter bf;
        file::deserialize(out_file_3, bf);

        genome::sample<31> s;
        file::deserialize(sample_9, s);
        for (auto kmer: s.data()) {
            ASSERT_TRUE(bf.contains(kmer, 8));
        }
    }

    TEST(bloom_filter, two_outputs) {
        boost::filesystem::remove_all(out_dir);
        genome::bloom_filter::process_all_in_directory(in_dir_3, out_dir, bloom_filter_size, block_size, num_hashes);

        bloom_filter bf;
        file::deserialize(out_file_4, bf);

        genome::sample<31> s;
        file::deserialize(sample_9, s);
        for (auto kmer: s.data()) {
            ASSERT_TRUE(bf.contains(kmer, 0));
        }
    }

    TEST(bloom_filter, false_positive) {
        boost::filesystem::remove_all(out_dir);
        genome::bloom_filter::process_all_in_directory(in_dir_2, out_dir, bloom_filter_size, block_size, num_hashes);

        bloom_filter bf;
        file::deserialize(out_file_2, bf);

        size_t num_tests = 100000;
        size_t num_positive = 0;
        for (size_t i = 0; i < num_tests; i++) {
            std::array<byte, 8> a = {(byte) (i >> 0), (byte) (i >> 8), (byte) (i >> 16), (byte) (i >> 24),
                                     (byte) (i >> 32), (byte) (i >> 40), (byte) (i >> 48), (byte) (i >> 56)};
            kmer<31> k(a);
            if (bf.contains(k, 0)) {
                num_positive++;
            }
        }
        ASSERT_EQ(num_positive, 945);
    }

    TEST(bloom_filter, equal_ones_and_zeros) {
        boost::filesystem::remove_all(out_dir);
        genome::bloom_filter::process_all_in_directory(in_dir_2, out_dir, bloom_filter_size, block_size, num_hashes);

        bloom_filter bf;
        file::deserialize(out_file_2, bf);

        size_t ones = 0;
        for (auto b: bf.data()) {
            ASSERT_TRUE(b == 0 || b == 1);
            ones += b;
        }
        size_t zeros = bf.data().size() - ones;
        ASSERT_EQ(zeros, 97734);
        ASSERT_EQ(ones, 102266);
    }

    TEST(bloom_filter, others_zero) {
        boost::filesystem::remove_all(out_dir);
        genome::bloom_filter::process_all_in_directory(in_dir_2, out_dir, bloom_filter_size, block_size, num_hashes);

        bloom_filter bf;
        file::deserialize(out_file_2, bf);

        size_t num_positive = 0;
        for (size_t i = 0; i < 100000; i++) {
            std::array<byte, 8> a = {(byte) (i >> 0), (byte) (i >> 8), (byte) (i >> 16), (byte) (i >> 24),
                                     (byte) (i >> 32), (byte) (i >> 40), (byte) (i >> 48), (byte) (i >> 56)};
            kmer<31> k(a);
            ASSERT_FALSE(bf.contains(k, i % 5 + 3));
        }
    }
}
