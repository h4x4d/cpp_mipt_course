#pragma once
#include <vector>

template <size_t N, size_t M, typename T = int64_t>
class Matrix {
 public:
  Matrix() : Matrix(T()){};

  Matrix(const std::vector<std::vector<T>>& vector) : matrix_(vector){};

  Matrix(const Matrix<N, M, T>& matrix) : Matrix(matrix.matrix_){};

  Matrix(const T& elem)
      : matrix_(std::vector<std::vector<T>>(N, std::vector<T>(M, elem))){};

  Matrix& operator+=(const Matrix& other);

  Matrix& operator-=(const Matrix& other);

  Matrix& operator+=(T element);

  Matrix& operator-=(T element) { return (*this += (-element)); };

  Matrix& operator*=(T element);

  T& operator()(size_t row, size_t col) { return matrix_[row][col]; };

  const T& operator()(size_t row, size_t col) const {
    return matrix_[row][col];
  };

  Matrix<M, N, T> Transposed();

  T Trace();

 private:
  std::vector<std::vector<T>> matrix_;
};

template <size_t N, size_t M, typename T = int64_t>
Matrix<N, M, T> operator+(const Matrix<N, M, T>& first,
                          const Matrix<N, M, T>& second) {
  Matrix<N, M, T> result(first);
  result += second;
  return result;
}

template <size_t N, size_t M, typename T = int64_t>
Matrix<N, M, T> operator+(const Matrix<N, M, T>& first, T element) {
  Matrix<N, M, T> result(first);
  result += element;
  return result;
}

template <size_t N, size_t M, typename T = int64_t>
Matrix<N, M, T> operator-(const Matrix<N, M, T>& first,
                          const Matrix<N, M, T>& second) {
  Matrix<N, M, T> result(first);
  result -= second;
  return result;
}

template <size_t N, size_t M, typename T = int64_t>
Matrix<N, M, T> operator-(const Matrix<N, M, T>& first, T element) {
  Matrix<N, M, T> result(first);
  result -= element;
  return result;
}

template <size_t N, size_t M, typename T = int64_t>
Matrix<N, M, T> operator*(const Matrix<N, M, T>& first, T element) {
  Matrix<N, M, T> result(first);
  result *= element;
  return result;
}

template <size_t N, size_t M, size_t K, typename T = int64_t>
Matrix<N, K, T> operator*(const Matrix<N, M, T>& first,
                          const Matrix<M, K, T>& second) {
  Matrix<N, K, T> result;
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < K; ++j) {
      for (size_t k = 0; k < M; ++k) {
        result(i, j) += first(i, k) * second(k, j);
      }
    }
  }
  return result;
}

template <size_t N, size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator+=(const Matrix<N, M, T>& other) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      matrix_[i][j] += other(i, j);
    }
  }
  return *this;
};

template <size_t N, size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator-=(const Matrix<N, M, T>& other) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      matrix_[i][j] -= other(i, j);
    }
  }
  return *this;
};

template <size_t N, size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator+=(T element) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      matrix_[i][j] += element;
    }
  }
  return *this;
};

template <size_t N, size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator*=(T element) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      matrix_[i][j] *= element;
    }
  }
  return *this;
};

template <size_t N, size_t M, typename T>
Matrix<M, N, T> Matrix<N, M, T>::Transposed() {
  Matrix<M, N, T> result;
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      result(j, i) = matrix_[i][j];
    }
  }
  return result;
};

template <size_t N, size_t M, typename T>
T Matrix<N, M, T>::Trace() {
  static_assert(N == M);
  T result = 0;
  for (size_t i = 0; i < N; ++i) {
    result += matrix_[i][i];
  }
  return result;
};