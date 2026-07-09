//
// Created by carlo on 2026-01-22.
//

#ifndef CODESTRUCTS_CUH
#define CODESTRUCTS_CUH

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <string>

namespace CodeCuda
{

    constexpr int32_t k_matrix_pretty_print_limit = 128;
    constexpr int32_t k_matrix_summary_edge_count = 1;

    inline std::string FormatMatrixValue(float value)
    {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(3) << value;
        std::string formatted = stream.str();

        if (formatted.find('.') != std::string::npos)
        {
            while (!formatted.empty() && formatted.back() == '0')
            {
                formatted.pop_back();
            }
            if (!formatted.empty() && formatted.back() == '.')
            {
                formatted.pop_back();
            }
        }

        if (formatted == "-0")
        {
            return "0";
        }

        return formatted;
    }

    inline std::string FormatMatrixOutput(int32_t M, int32_t N, const float *data, bool force_full_output = false)
    {
        std::ostringstream output;
        output << "Shape: " << M << " x " << N << '\n';

        if (data == nullptr || M <= 0 || N <= 0)
        {
            output << "[]\n\n";
            return output.str();
        }

        const bool summarize_output = !force_full_output && (M * N) > k_matrix_pretty_print_limit;

        int32_t cell_width = 0;
        for (int32_t i = 0; i < M * N; ++i)
        {
            cell_width = std::max<int32_t>(cell_width, static_cast<int32_t>(FormatMatrixValue(data[i]).size()));
        }

        cell_width = std::max<int32_t>(cell_width, 2); // for "..."

        const auto should_print_row = [M, summarize_output](int32_t row)
        {
            if (!summarize_output || M <= k_matrix_summary_edge_count * 2)
                return true;

            return row < k_matrix_summary_edge_count || row >= M - k_matrix_summary_edge_count;
        };

        const auto should_print_col = [N, summarize_output](int32_t col)
        {
            if (!summarize_output || N <= k_matrix_summary_edge_count * 2)
                return true;

            return col < k_matrix_summary_edge_count || col >= N - k_matrix_summary_edge_count;
        };

        output << "[\n";

        bool printed_row_gap = false;

        for (int32_t row = 0; row < M; ++row)
        {
            if (!should_print_row(row))
            {
                if (!printed_row_gap)
                {
                    output << "  ...\n";
                    printed_row_gap = true;
                }
                continue;
            }

            printed_row_gap = false;

            output << "  [ ";

            bool first_cell = true;
            bool printed_col_gap = false;

            for (int32_t col = 0; col < N; ++col)
            {
                if (!should_print_col(col))
                {
                    if (!printed_col_gap)
                    {
                        if (!first_cell)
                            output << "  ";

                        output << std::setw(cell_width) << "...";

                        first_cell = false;
                        printed_col_gap = true;
                    }

                    continue;
                }

                if (!first_cell)
                    output << "  ";

                output << std::setw(cell_width) << FormatMatrixValue(data[row * N + col]);

                first_cell = false;
            }

            output << " ]\n";
        }

        output << "]\n\n";
        return output.str();
    }
    inline std::string FormatMatrixOutputNoPadding(int32_t M, int32_t N, const float *data,
                                                   bool force_full_output = false)
    {
        std::ostringstream output;
        output << "Shape: " << M << " x " << N << '\n';

        if (data == nullptr || M <= 0 || N <= 0)
        {
            output << "[]\n\n";
            return output.str();
        }

        const bool summarize_output = !force_full_output && (M * N) > k_matrix_pretty_print_limit;

        int32_t cell_width = 0;
        for (int32_t i = 0; i < M * N; ++i)
        {
            cell_width = std::max<int32_t>(cell_width, static_cast<int32_t>(FormatMatrixValue(data[i]).size()));
        }

        const auto should_print_row = [M, summarize_output](int32_t row)
        {
            if (!summarize_output || M <= (k_matrix_summary_edge_count * 2))
            {
                return true;
            }
            return row < k_matrix_summary_edge_count || row >= (M - k_matrix_summary_edge_count);
        };

        const auto should_print_col = [N, summarize_output](int32_t col)
        {
            if (!summarize_output || N <= (k_matrix_summary_edge_count * 2))
            {
                return true;
            }
            return col < k_matrix_summary_edge_count || col >= (N - k_matrix_summary_edge_count);
        };

        output << "[\n";
        bool skipped_rows = false;
        for (int32_t row = 0; row < M; ++row)
        {
            if (!should_print_row(row))
            {
                if (!skipped_rows)
                {
                    output << "  ...\n";
                    skipped_rows = true;
                }
                continue;
            }

            skipped_rows = false;
            output << "  [ ";
            bool skipped_cols = false;
            for (int32_t col = 0; col < N; ++col)
            {
                if (!should_print_col(col))
                {
                    if (!skipped_cols)
                    {
                        if (col > 0)
                        {
                            output << "  ";
                        }
                        output << "...";
                        skipped_cols = true;
                    }
                    continue;
                }

                if (col > 0)
                {
                    output << " ";
                }

                if (skipped_cols)
                {
                    skipped_cols = false;
                }

                output << FormatMatrixValue(data[row * N + col]);
            }
            output << " ]\n";
        }
        output << "]\n\n";
        return output.str();
    }
    struct c_matrix
    {
        c_matrix(const c_matrix &) = delete;
        c_matrix &operator=(const c_matrix &) = delete;
        c_matrix(int32_t M, int32_t N)
        {
            this->M = M;
            this->N = N;
            this->data_size = this->M * this->N;
            this->data = new float[M * N];
            this->data_t = new float[M * N];
            this->shape = new int[2];
            this->shape[0] = M;
            this->shape[1] = N;
        }
        c_matrix &Full(float val)
        {
            for (int32_t i = 0; i < this->data_size; i++)
            {
                data[i] = val;
            }
            return *this;
        }

        c_matrix &Rand()
        {
            srand(40);
            for (int32_t i = 0; i < this->data_size; i++)
            {
                data[i] = rand() % 1000;
            }
            return *this;
        }

        c_matrix &Full_Arange()
        {
            for (int32_t i = 0; i < this->data_size; i++)
            {
                data[i] = float(i);
            }
            return *this;
        }

        c_matrix &RandInt(int min, int max)
        {
            srand(40);
            for (int32_t i = 0; i < this->data_size; i++)
            {
                data[i] = float(min + (rand() % (max - min)));
            }
            return *this;
        }
        c_matrix &Rand(int min, int max)
        {
            srand(40);
            for (int32_t i = 0; i < this->data_size; i++)
            {
                float decimal = float(float(rand()) / float(RAND_MAX));
                data[i] = float(min + (rand() % (max - min))) + decimal;
            }
            return *this;
        }

        c_matrix &Arange_Sequential()
        {
            for (int32_t i = 0; i < this->data_size; i++)
            {
                data[i] = i;
            }
            return *this;
        }
        c_matrix &BuildTranpose()
        {
            for (int32_t i = 0; i < this->data_size; i++)
            {
                int32_t x = int32_t(i % N);
                int32_t y = int32_t(i / N);
                data_t[x * this->M + y] = data[i];
            }
            return *this;
        }
        c_matrix &Print(bool force_full_output = false, bool padded = true)
        {
            const std::string output = (padded) ? FormatMatrixOutput(M, N, data, force_full_output)
                                                : FormatMatrixOutputNoPadding(M, N, data, force_full_output);
            printf("%s", output.c_str());
            return *this;
        }
        ~c_matrix()
        {
            delete[] shape;
            delete[] data_t;
            delete[] data;
        }
        float *Get_Data() const
        {
            assert(data != nullptr);
            return data;
        }
        float *Get_T()
        {
            assert(data_t != nullptr);
            BuildTranpose();
            return data_t;
        }

        int32_t *Shape() { return shape; }
        static void PrintAnyMatrix(int32_t M, int32_t N, float *data, bool force_full_output = false,
                                   bool padded = true)
        {
            const std::string output = padded ? FormatMatrixOutput(M, N, data, force_full_output)
                                              : FormatMatrixOutputNoPadding(M, N, data, force_full_output);

            printf("%s", output.c_str());
        }

    private:
        float *data = nullptr;
        float *data_t = nullptr;
        int32_t M = 0;
        int32_t N = 0;
        int32_t *shape = nullptr;
        int32_t data_size = 0;
    };

} // namespace CodeCuda


#endif // CODESTRUCTS_CUH
