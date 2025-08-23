#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>
#include "MiniscriptTypes.h"
#include "SplitJoin.h"

using namespace MiniScript;

// Global word arrays - equivalent to MiniScript's split arrays
StringList singles;
StringList teens;
StringList tys;
StringList ions;

void initializeWordArrays() {
    // singles = " one two three four five six seven eight nine ".split
    singles = Split(String(" one two three four five six seven eight nine "), String(" "));
    
    // teens = "ten eleven twelve thirteen fourteen fifteen sixteen seventeen eighteen nineteen ".split
    teens = Split(String("ten eleven twelve thirteen fourteen fifteen sixteen seventeen eighteen nineteen "), String(" "));
    
    // tys = "  twenty thirty forty fifty sixty seventy eighty ninety".split
    tys = Split(String("  twenty thirty forty fifty sixty seventy eighty ninety"), String(" "));
    
    // ions = "thousand million billion".split
    ions = Split(String("thousand million billion"), String(" "));
}

String numberToText(long n) {
    if (n == 0) return String("zero");
    
    long a = std::abs(n);
    String r = String("");  // result
    
    // Process each scale (units, thousands, millions, billions)
    for (long ionIndex = 0; ionIndex < (long)ions.Count(); ionIndex++) {
        String u = ions[ionIndex];
        
        long h = a % 100;
        if (h > 0 && h < 10) {
            r = singles[h] + String(" ") + r;
        }
        if (h > 9 && h < 20) {
            r = teens[h-10] + String(" ") + r;
        }
        if (h > 19 && h < 100) {
            String hyphen = (h % 10 > 0) ? String("-") : String("");
            r = tys[h/10] + hyphen + singles[h%10] + String(" ") + r;
        }
        
        h = (a % 1000) / 100;
        if (h) {
            r = singles[h] + String(" hundred ") + r;
        }
        
        a = a / 1000;
        if (a == 0) break;
        if (a % 1000 > 0) {
            r = u + String(" ") + r;
        }
    }
    
    if (n < 0) {
        r = String("negative ") + r;
    }
    
    return r.Trim();
}

long textToNumber(String s) {
    if (s == String("zero")) return 0;
    
    // Replace hyphens with spaces and split
    String cleanS = s.Replace("-", " ");
    StringList words = Split(cleanS, String(" "));
    
    long result = 0;
    long ionVal = 0;
    bool negative = false;
    long maxi = words.Count() - 1;
    long i = 0;
    
    while (i <= maxi) {
        String word = words[i];
        
        if (word == String("negative")) {
            negative = true;
            i++;
            continue;
        }
        
        // Check for scale words (thousand, million, billion)
        long idx = -1;
        for (long j = 0; j < (long)ions.Count(); j++) {
            if (ions[j] == word) {
                idx = j;
                break;
            }
        }
        
        if (idx != -1) {
            long multipliers[] = {1000, 1000000, 1000000000};
            result += ionVal * multipliers[idx];
            ionVal = 0;
            i++;
            continue;
        }
        
        long wordVal = 0;
        
        // Check singles
        idx = -1;
        for (long j = 0; j < (long)singles.Count(); j++) {
            if (singles[j] == word) {
                idx = j;
                break;
            }
        }
        
        if (idx != -1) {
            wordVal = idx;
        } else {
            // Check tys (twenties, thirties, etc.)
            idx = -1;
            for (long j = 0; j < (long)tys.Count(); j++) {
                if (tys[j] == word) {
                    idx = j;
                    break;
                }
            }
            
            if (idx != -1) {
                wordVal = idx * 10;
            } else {
                // Check teens
                idx = -1;
                for (long j = 0; j < (long)teens.Count(); j++) {
                    if (teens[j] == word) {
                        idx = j;
                        break;
                    }
                }
                
                if (idx != -1) {
                    wordVal = idx + 10;
                } else {
                    std::cout << "Unexpected word: " << word.c_str() << std::endl;
                    return 0;
                }
            }
        }
        
        // Handle "hundred"
        if (i < maxi && words[i+1] == String("hundred")) {
            wordVal *= 100;
            i++;
        }
        
        ionVal += wordVal;
        i++;
    }
    
    result += ionVal;
    if (negative) result = -result;
    
    return result;
}

double get_time() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
    return nanoseconds.count() / 1000000000.0;
}

void runBenchmark(long n = 10000) {
    double t0 = get_time();
    
    for (long i = 0; i < n; i++) {
        String s = numberToText(i);
        long i2 = textToNumber(s);
        if (i2 != i) {
            std::cout << "Oops! Failed on " << i << ":" << std::endl;
            std::cout << s.c_str() << " --> " << i2 << std::endl;
        }
    }
    
    double t1 = get_time();
    double elapsed = t1 - t0;
    
    std::cout << "numberWords(" << n << ") time: " << elapsed << " seconds" << std::endl;
}

void runCorrectnessTests() {
    std::cout << "Correctness checks:" << std::endl;
    
    long testNumbers[] = {-1234, 0, 7, 42, 4325, 1000004, 214837564};
    int numTests = sizeof(testNumbers) / sizeof(testNumbers[0]);
    
    for (int i = 0; i < numTests; i++) {
        long n = testNumbers[i];
        String words = numberToText(n);
        long backToNum = textToNumber(words);
        
        std::cout << n << ": " << words.c_str() << " -> " << backToNum;
        if (backToNum != n) {
            std::cout << " ERROR --^" << std::endl;
            return;
        } else {
            std::cout << std::endl;
        }
    }
}

int main() {
    std::cout << "MiniScript::Value NumberWords Benchmark" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    initializeWordArrays();
    
    runCorrectnessTests();
    std::cout << std::endl;
    
    runBenchmark();
    
    return 0;
}