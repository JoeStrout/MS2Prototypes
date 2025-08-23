#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include "MiniscriptTypes.h"

using namespace MiniScript;

// Implement Levenshtein distance algorithm using MiniScript::Value
Value editDistance(Value s1, Value s2) {
    if (s1.type != ValueType::String || s2.type != ValueType::String) {
        return Value::null;
    }
    
    String str1 = s1.GetString();
    String str2 = s2.GetString();
    
    long n = str1.Length();
    long m = str2.Length();
    
    if (n == 0) return Value(static_cast<double>(m));
    if (m == 0) return Value(static_cast<double>(n));
    
    // Split strings into character arrays - manually since we need individual characters
    ValueList s1chars;
    ValueList s2chars;
    
    // Convert strings to character lists
    for (long i = 0; i < n; i++) {
        String ch = str1.Substring(i, 1);
        s1chars.Add(Value(ch));
    }
    
    for (long i = 0; i < m; i++) {
        String ch = str2.Substring(i, 1);
        s2chars.Add(Value(ch));
    }
    
    // Initialize distance array - equivalent to d = range(0, m)
    std::vector<long> d(m + 1);
    for (long i = 0; i <= m; i++) {
        d[i] = i;
    }
    
    long lastCost = 0;
    
    // Main algorithm loop
    for (long i = 1; i <= n; i++) {
        String s1char = s1chars[i-1].GetString();
        lastCost = i;
        long jMinus1 = 0;
        
        for (long j = 1; j <= m; j++) {
            long cost = (s1char == s2chars[jMinus1].GetString()) ? 0 : 1;
            
            // Calculate minimum of three possibilities
            long a = d[j] + 1;          // deletion
            long b = lastCost + 1;      // insertion  
            long c = cost + d[jMinus1]; // substitution
            
            long nextCost;
            if (a < b) {
                nextCost = (c < a) ? c : a;
            } else {
                nextCost = (c < b) ? c : b;
            }
            
            d[jMinus1] = lastCost;
            lastCost = nextCost;
            jMinus1 = j;
        }
        d[m] = lastCost;
    }
    
    return Value(static_cast<double>(lastCost));
}

double get_time() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
    return nanoseconds.count() / 1000000000.0;
}

void runTest() {
    // Test case 1: "kitten" vs "sitting" (expected: 3)
    Value result1 = editDistance(Value("kitten"), Value("sitting"));
    std::cout << "editDistance(\"kitten\", \"sitting\") = " << result1.IntValue() << std::endl;
    
    // Test case 2: similar sentences
    String s1 = "this is a test of a slightly longer string";
    String s2 = "that was a test of a slightly longer string";
    Value result2 = editDistance(Value(s1), Value(s2));
    std::cout << "editDistance(sentence1, sentence2) = " << result2.IntValue() << std::endl;
    
    // Gettysburg Address variants (large test)
    String ga1 = "Four score and seven years ago our fathers brought forth on this continent, a new nation, conceived in Liberty, and dedicated to the proposition that all men are created equal.  Now we are engaged in a great civil war, testing whether that nation, or any nation so conceived and so dedicated, can long endure. We are met on a great battle-field of that war. We have come to dedicate a portion of that field, as a final resting place for those who here gave their lives that that nation might live. It is altogether fitting and proper that we should do this. " +
    String("But, in a larger sense, we can not dedicate—we can not consecrate—we can not hallow—this ground. The brave men, living and dead, who struggled here, have consecrated it, far above our poor power to add or detract. The world will little note, nor long remember what we say here, but it can never forget what they did here. It is for us the living, rather, to be dedicated here to the unfinished work which they who fought here have thus far so nobly advanced. ") +
    String("It is rather for us to be here dedicated to the great task remaining before us—that from these honored dead we take increased devotion to that cause for which they gave the last full measure of devotion—that we here highly resolve that these dead shall not have died in vain—that this nation, under God, shall have a new birth of freedom—and that government of the people, by the people, for the people, shall not perish from the earth.");
    
    String ga2 = "Eighty seven years ago our ancestors brought forth in these parts, a new nation, conceived in freedom, and dedicated to the proposition that all people are created equal.  Now we are engaged in a lousy civil war, testing whether that nation, or any nation so conceived and so dedicated, can long endure. We are gathered on a famous battlefield of that war. We have come to dedicate a portion of that field, as a final resting place for those who here gave their lives that that nation might live. It is super groovy and cool that we should do this. " +
    String("But, in a larger sense, we can not dedicate — we can not consecrate — we can not hallow — this ground. The brave men, living and dead, who struggled here, have consecrated it, far above our poor power to add or subtract. The world will little note, nor long remember what we say here (ha ha as if), but it can never forget what they did here. It is for us the living, rather, to be dedicated here to the unfinished work which they who fought here have thus far so nobly advanced. ") +
    String("It is rather for us to be here dedicated to the great task remaining before us — that from these honored dead we take increased devotion to that cause for which they gave the last full measure of devotion — that we here highly resolve that these dead shall not have died in vain — that this nation, with its constitutionally guaranteed separation of church and state, shall have a new birth of freedom — and that government of the people, by the people, for the people, shall not disappear from Earth.");
    
    // Test case 3: Gettysburg variants
    Value result3 = editDistance(Value(ga1), Value(ga2));
    std::cout << "editDistance(gettysburg1, gettysburg2) = " << result3.IntValue() << std::endl;
    
    // Test case 4: Large vs small string
    Value result4 = editDistance(Value(ga1), Value("banana"));
    std::cout << "editDistance(gettysburg, \"banana\") = " << result4.IntValue() << std::endl;
}

void runBenchmark() {
    double t0 = get_time();
    runTest();
    double t1 = get_time();
    
    double elapsed = t1 - t0;
    std::cout << "levenshtein: " << elapsed << " seconds" << std::endl;
}

int main() {
    std::cout << "MiniScript::Value Levenshtein Benchmark" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    runBenchmark();
    
    return 0;
}