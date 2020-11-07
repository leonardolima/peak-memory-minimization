#include <iostream>
#include <fstream>
#include <vector>
#include <Eigen/Dense>
#include <chrono>
#include "test.h"
#include "adapted_cuthill_mckee.h"
#include "level.h"
#include "topological.h"
#include "io.h"
#include "basic.h"

/*******************************************************************************
 * Lists each integer of a particular string.
 *
 *
 * @param s String.
 * @param del Delimiter between integers.
 ******************************************************************************/
std::vector<int> string_split(const std::string& s, const char del)
{
    std::vector<int> tokens;
    std::string token;
    std::istringstream token_stream(s);

    while(std::getline(token_stream, token, del))
    {
        tokens.push_back(std::stoi(token));
    }

    return tokens;
}

int read_N_from_file (const std::string& file_name)
{
    std::ifstream file(file_name);
    if (!file.is_open()) return -1;

    std::string line;
    std::getline(file, line);
    int N = std::stoi(line);

    file.close();

    return N;
}

/*******************************************************************************
 * From the input file, generates a matrix A representing the graph and a vector
 * O in order to check the ordering when applying the chosen algorithm.
 *
 * Input file has the following format:
 * First line corresponds to the number of nodes (N)
 * From the second line onwards:
 * n1 o1 (i.e., node 1 has output size o1)
 * n2 o2 n1 (i.e., node 2 has output size o2 and receives n1 as input)
 *
 * @param file_name String containing file's name.
 * @param A Adjacency matrix of the graph.
 * @param O Ordering constraints (given in the form L_x > [L_yi]).
 ******************************************************************************/
void read_file(const std::string& file_name, Eigen::MatrixXf& A, int N)
{
    std::vector<std::vector<int>> tokens(N), O(N);

    // File initialization
    std::ifstream file(file_name);

    if (!file.is_open()) return;

    // Read first line and ignore (we already know N)
    std::string line;
    std::getline(file, line);

    // Reading file line by line
    int i = 0;

    while (std::getline(file, line))
    {
        tokens[i] = string_split(line, ' ');

        A(i, i) = float(tokens[i][0]);

        for (std::vector<int>::size_type j = 1; j < tokens[i].size(); ++j)
        {
            O[i].push_back(tokens[i][j]);
        }

        i++;
    }

    // Update matrix A only after all data is read
    for (std::vector<std::vector<int>>::size_type i = 0; i < tokens.size(); ++i)
    {
        for (std::vector<int>::size_type j = 1; j < tokens[i].size(); ++j)
        {
            A(i, tokens[i][j]) = A(tokens[i][j], tokens[i][j]);
        }
    }

    file.close();
}

/*******************************************************************************
 * Changes constraints format.
 *
 * Constraints are given in the form L_x > [L_yi], but in order to keep search
 * O(1) the format is changed to L_x < [L_yi].
 *
 *
 * @param O     Ordering constraints (in the form L_x > [L_yi]).
 * @param new_O Ordering constraints (in the form L_x < [L_yi]).
 ******************************************************************************/
void convert_vector(const std::vector<std::vector<int>>& O,
                    std::vector<std::vector<int>>& new_O)
{
    for (std::vector<std::vector<int>>::size_type i = 0; i < O.size(); ++i)
    {
        for (std::vector<int>::size_type j = 0; j < O[i].size(); ++j)
        {
            new_O[O[i][j]].push_back(i);
        }
    }
}

void test_adapted_cuthill_mckee(const Eigen::MatrixXf& A, Eigen::MatrixXf& P, Eigen::MatrixXf& R,
                                std::vector<int>& path1, std::vector<int>& path2)
{
    std::cout << "-------------------------------------------" << std::endl;
    std::cout << "Applying adapted Cuthill-McKee approach: " << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    apply_adapted_cuthill_mckee(A, P, path1, path2);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<float>(stop - start);
    auto duration_s = std::chrono::duration_cast<std::chrono::seconds>(duration);
    std::cout << "Execution time = " << duration_s.count() << "s" << std::endl;
    std::cout << std::endl;

    R = (P*A*P.transpose());

    print_bandwidth_comparison(A, R);
    print_peak_comparison(A, path1, path2);
    std::cout << "-------------------------------------------" << std::endl;
}

void test_levels(const Eigen::MatrixXf& A, Eigen::MatrixXf& P, Eigen::MatrixXf& R,
                 std::vector<int>& path)
{
    std::cout << "-------------------------------------------" << std::endl;
    std::cout << "Applying level approach: " << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    apply_levels(A, P, path);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<float>(stop - start);
    auto duration_s = std::chrono::duration_cast<std::chrono::seconds>(duration);
    std::cout << "Execution time = " << duration_s.count() << "s" << std::endl;
    std::cout << std::endl;

    R = (P*A*P.transpose());

    print_bandwidth_comparison(A, R);
    print_peak_comparison(A, path, path);
    std::cout << "-------------------------------------------" << std::endl;
}

void test_topological(const Eigen::MatrixXf& A, Eigen::MatrixXf& P, Eigen::MatrixXf& R,
                      std::vector<int>& path1, std::vector<int>& path2)
{
    std::cout << "-------------------------------------------" << std::endl;
    std::cout << "Applying topological approach: " << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    apply_topological(A, P, path1, path2);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<float>(stop - start);
    auto duration_s = std::chrono::duration_cast<std::chrono::seconds>(duration);
    std::cout << "Execution time = " << duration_s.count() << "s" << std::endl;
    std::cout << std::endl;

    R = (P*A*P.transpose());

    print_bandwidth_comparison(A, R);
    print_peak_comparison(A, path1, path2);
    std::cout << "-------------------------------------------" << std::endl;
}

void test_all(const std::string& file_name)
{
    int N = read_N_from_file(file_name);

    // Adjacency matrix representing the graph
    Eigen::MatrixXf A = Eigen::MatrixXf::Identity(N, N);

    // Permutation matrix (in case of Cuthill-McKee algorithm)
    Eigen::MatrixXf P = Eigen::MatrixXf::Zero(N, N);

    // Resulting matrix
    Eigen::MatrixXf R = Eigen::MatrixXf::Zero(N, N);

    // Paths
    std::vector<int> path1;
    std::vector<int> path2;

    read_file(file_name, A, N);

    // 1. Topological sorting
    // test_topological(A, P, R, path1, path2);
    // path1.clear();
    // path2.clear();

    // 2. Level approach
    test_levels(A, P, R, path1);
    path1.clear();

    // 3. Adapted Cuthill-McKee approach
    test_adapted_cuthill_mckee(A, P, R, path1, path2);
    path1.clear();
    path2.clear();
}
