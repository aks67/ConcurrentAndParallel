#include <iostream>
#include <type_traits>
#include <iterator>
#include <thread>
#include <algorithm>
#include <vector>
#include <numeric> 
#include <functional>
#include <chrono>
#include <random>

template<typename Iterator, typename T> 
struct accumulate_block {
    void operator()(Iterator first, Iterator last, T& result) {
        result = std::accumulate(first, last, result);
    }
};

template<typename Iterator, typename T> 
T parallel_accumulate(Iterator first, Iterator last, T init) {
    unsigned long const length = std::distance(first, last);

    if (length == 0) return init;

    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;

    unsigned long const hardware_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;

    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads - 1);

    Iterator block_start = first;
    for (unsigned long i = 0; i < (num_threads - 1); ++i) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        threads[i] = std::thread(accumulate_block<Iterator, T>(), block_start, block_end, std::ref(results[i]));
        block_start = block_end;
    }

    accumulate_block<Iterator, T>()(block_start, last, results[num_threads - 1]);
    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

    return std::accumulate(results.begin(), results.end(), init);
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());

    int n = 10000000; 
    std::vector<int> numbers(n);
    std::uniform_int_distribution<> dis(1, 1000);
    for (int i = 0; i < n; ++i) {
        numbers[i] = dis(gen);
    }


    auto start1 = std::chrono::steady_clock::now();
    int totl = std::accumulate(numbers.begin(), numbers.end(), 0);
    auto end1 = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed1 = end1 - start1;

    // Timing for parallel_accumulate
    auto start2 = std::chrono::steady_clock::now();
    int sum = parallel_accumulate(numbers.begin(), numbers.end(), 0);
    auto end2 = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed2 = end2 - start2;

    // Output the result and timing information
    std::cout << "Sum of numbers (std::accumulate): " << totl << std::endl;
    std::cout << "Time taken for std::accumulate: " << elapsed1.count() << " seconds" << std::endl;

    std::cout << "Sum of numbers (parallel_accumulate): " << sum << std::endl;
    std::cout << "Time taken for parallel_accumulate: " << elapsed2.count() << " seconds" << std::endl;

    return 0;
}