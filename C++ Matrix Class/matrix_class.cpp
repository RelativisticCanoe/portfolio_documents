/**
 * Assignment 5: Matrix Class and Copy/Move Construction and Assignment
 * Written by: James Michael Patrick Brady
 * 
 * This program defines a Matrix class for multidimensional arithmetic, using doubles as
 * the stored data type. The Matrix contains three attributes: the number of rows, the
 * number of columns and the data of the matrix (stored as a pointer to a 1D double array
 * with an initially undefined length, allowing for matricies of any size).
 * 
 * A copy constructor and copy assignment operator is defined for the matrix class when
 * being constructed/assigned from a lvalue matrix. Likewise, a move constructor and
 * move assignment operator is defined when rvalue matricies are used.
 * 
 * Addition, subtraction and multiplication (both using a scalar and another matrix) have
 * been overridden to work with the matrix class as expected. There are protections as needed
 * to prevent illegal operations and in the case that one is attempted, a 1x1 matrix with value
 * NaN is returned, as this can be easily caught in a program loop.
 * 
 * The I/O operators have also been overloaded so that a matrix can be natively output to the
 * terminal and that either an istream or a 1D array of doubles can be used to define the data
 * that a matrix contains.
 * 
 * The matrix also includes a method to remove the ith row and jth column of a matrix, returning a
 * separate matrix (and hence not destroying the original) as well as a method that calculates the determinant
 * of the matrix recursively using the remove() method.
 * 
 * The main() function tests all of these capabilities.
 **/

//Headers
#include <iostream>
#include <cmath>
#include <array>

//Classes
class matrix
{
    private:
        //If, for whatever reason, the ints are not assigned they will default to 1
        int rows{1};
        int columns{1};
        //Initialisation to NULL is generally a good idea
        double* matrix_data{nullptr};

    public:
        matrix(int m, int n)
        {
            /* Constructs a Matrix with the specified number of rows and columns, with
            all values initialised to 0. */
            rows = m;
            columns = n;
            int length{rows * columns};
            matrix_data = new double[length];
            //Just initialises the matrix to store a value of 0 at all positions, as a precaution.
            for(int i = 0; i < length; i++){
                matrix_data[i] = 0;
            }
        }

        ~matrix()
        {
            //Never forget this!
            delete[] matrix_data;
        }

        //Copy constructor and copy assignment operator
        matrix(matrix& mat)
        {
            //I use this-> when multiple matrices are used at once on one line, to keep everything clear
            //When using this object only, the this-> is skipped
            this->rows = mat.rows;
            this->columns = mat.columns;
            size_t length{rows * columns};
            matrix_data = new double[length];
            for(int i = 0; i < length; i++){
                this->matrix_data[i] = mat.matrix_data[i]; 
            }
        }

        matrix& operator= (matrix& mat)
        {   
            /* Handles assignment by copying a matrix. BROKEN */
            //Self assignment handled by returning self
            if(&mat == this){
                return *this;
            } else{
                size_t length{mat.rows * mat.columns};
                if(length > 0){
                    this->rows = mat.rows;
                    this->columns = mat.columns;
                    matrix_data = new double[length];
                    for(size_t i{0}; i < length; i++){
                        this->matrix_data[i] = mat.matrix_data[i];
                    }
                }
                return *this;    
            }
        }

        //Move constructor and move assignment operator
        matrix(matrix&& mat)
        {
            this->rows = mat.rows;
            this->columns = mat.columns;
            this->matrix_data = mat.matrix_data;
            mat.rows = 0;
            mat.columns = 0;
            mat.matrix_data = nullptr;
        }

        matrix& operator= (matrix&& mat)
        {
            //Uses swap to avoid both deep copying and referring to the same data
            std::swap(this->rows, mat.rows);
            std::swap(this->columns, mat.columns);
            std::swap(this->matrix_data, mat.matrix_data);
            return *this;
        }

        //Arithmetic and I/O Operators
        friend matrix operator+ (const matrix& a, const matrix& b);
        friend matrix operator- (const matrix& a, const matrix& b);
        friend matrix operator* (const matrix& a, const matrix& b);
        friend matrix operator* (const double& multiplier, const matrix& a);
        friend std::istream& operator>> (std::istream& input, matrix& mat);
        friend matrix& operator>> (double input[], matrix& mat);
        friend std::ostream& operator<< (std::ostream& output, const matrix& mat);
        
        //Getter functions for the rows, columns and lengrh
        const int get_rows() const
        {
            return rows;
        }

        const int get_columns() const
        {
            return columns;
        }

        const int get_length() const
        {
            return (rows * columns);
        }
        
        const double get_element(int i, int j)
        {
            return matrix_data[((i-1)*columns + j - 1)];
        }

        const matrix remove(int row_remove, int column_remove)
        {
            /* Returns a matrix that is a copy of this matrix, with the ith row and jth column removed.
            Const has been used at the start of the declaration as the resulting matrix should only
            depend on the data of the base matrix. */
            int i_remove{row_remove - 1};
            int j_remove{column_remove - 1};
            if((abs(i_remove) > (rows - 1))|| (abs(j_remove) > (columns - 1))){
                std::cout << "Indices out of range. Cannot remove row " << i_remove + 1 <<
                 " and column " << j_remove + 1 << " ." << std::endl;
                return *this;
            } else{
                //Adds in functionality for negative indices as with arrays (very handy for big matrices!)
                if(i_remove < 0){
                    i_remove += rows; 
                } else if(j_remove < 0){
                    j_remove += columns;
                } else{
                    matrix reduced(rows - 1, columns - 1);
                    size_t reduced_length{(rows - 1) * (columns - 1)};
                    int reduced_index{0};
                    int index{0};
                    for(int i{0}; i < rows; i++){
                        for(int j{0}; j < columns; j++){
                            if(!(j == j_remove) && !(i == i_remove))
                            {
                                index = j + (i * columns);
                                reduced.matrix_data[reduced_index] = matrix_data[index];
                                reduced_index++;
                            }
                        }
                    }
                    return reduced; 
                }
            }
        }

        const double determinant()
        {
            /* Calculates the determinant of the matrix recursively, provided it is a square matrix.
            This is achieved by using the minors method for any matrix of size above 2x2, down to
            the point that a 2x2 matrix is used, which the determinant for a 2x2 matrix is used.
            Const has been used before the type as this should only depend on the matrix's data.
            This cannot be followed by const as the data must be accessed by remove(). */
            if(rows == 2 && columns == 2)
            {
                return ((matrix_data[0] * matrix_data[3]) - (matrix_data[1] * matrix_data[2]));
            //Only works on square matrices
            } else if (rows == columns){
                double determinant_sum{0};
                for(int j{0}; j < rows; j++){
                    matrix minor_matrix{this->remove(1,j+1)};
                    if(j % 2 == 0){
                        determinant_sum = determinant_sum + (matrix_data[j] * minor_matrix.determinant());
                    } else{
                        determinant_sum = determinant_sum - (matrix_data[j] * minor_matrix.determinant());
                    }
                }
                return determinant_sum;
            } else{
                //A NaN is returned when the determinant cannot be calculated - no numerical answer is available
                std::cout << "Incompatible shape for determinant calculation: (" << rows << "x" <<
                columns << ") " << std::endl;
                double determinant_sum{std::nan("")};
            }
        }
};

//Class Operators
matrix operator+ (const matrix& a, const matrix& b)
{
    /* Checks that the two matricies have the same dimensions then sums them if they do
    and returns a 1x1 matrix with a NaN value if they don't, as a precaution*/
    if (a.rows == b.rows && a.columns == b.columns){
        matrix result(a.rows, a.columns);
        for(int i = 0; i < (a.rows * a.columns); i++){
            result.matrix_data[i] = a.matrix_data[i] + b.matrix_data[i];
        }
        return result;
    } else{
        //This is a layer of inbuilt protection against illegal operations
        std::cout << "Incompatible matrix sizes for addition: (" << a.rows << "x" << a.columns << ") and (" << b.rows 
        << "x" << b.columns << ")." << std::endl;
        matrix result(1,1);
        result.matrix_data[0] = std::nan("");
        return result;
    }
}

matrix operator- (const matrix& a, const matrix& b)
{
    /* As with the above + operation, but performs a subtraction instead */
    if (a.rows == b.rows && a.columns == b.columns){
        matrix result(a.rows, a.columns);
        for(int i = 0; i < (a.rows * a.columns); i++){
            result.matrix_data[i] = a.matrix_data[i] - b.matrix_data[i];
        }
        return result;
    } else{
        std::cout << "Incompatible matrix sizes for subtraction: (" << a.rows << "x" << a.columns << ") and (" << b.rows 
        << "x" << b.columns << ")." << std::endl;
        matrix result(1,1);
        result.matrix_data[0] = std::nan("");
        return result;
    }
}

matrix operator* (const matrix& a, const matrix& b)
{
    /* Checks that the number of columns in the left matrix is equal to the number of
    rows in the right matrix. If this is true, the dot product is performed by creating
    a new matrix and assigning values via nested for loops. If it is false, the previous
    1x1 NaN matrix is returned. */
    if (a.columns == b.rows){
        matrix result(a.rows, b.columns);
        int a_row{0};
        int b_column{0};
        for(int a_row = 0; a_row < a.rows; a_row++){
            for(int b_column = 0; b_column < b.columns; b_column++){
                double sum{0};
                int j{0};
                for(int i = 0; i < b.rows; i++){
                    int a_index{a_row*a.columns + j};
                    int b_index{i*b.columns + b_column};
                    double a_element{a.matrix_data[a_index]};
                    double b_element{b.matrix_data[b_index]};
                    sum += a_element * b_element;
                    j++;
                }
            int result_index{a_row*b.columns + b_column};
            result.matrix_data[result_index] = sum;
            }
        }
        return result;
    } else{
        std::cout << "Incompatible matrix sizes for dot product: (" << a.rows << "x" << a.columns << ") and (" << b.rows 
        << "x" << b.columns << ")." << std::endl;
        matrix result(1,1);
        result.matrix_data[0] = std::nan("");
        return result;
    }
}

matrix operator* (const double& multiplier, const matrix& a)
{
    /* Performs a scalar multiplication on a matrix, because it can be quite useful. */
    matrix result(a.rows, a.columns);
    for(int i = 0; i < a.rows * a.columns; i++)
    {
        result.matrix_data[i] = a.matrix_data[i] * multiplier;
    }
    return result;
}

std::istream& operator>> (std::istream& input, matrix& mat)
{
    /* Handles input by assigning doubles into a matrix until it has been filled.
    Non-double values will automatically trigger a cin.fail() state, which can
    be caught outside of the operator as required by a program. */
    double temp;
    for(int i{0}; i < (mat.rows * mat.columns); i++){
        input >> temp;
        mat.matrix_data[i] = temp;
    }
}

matrix& operator>> (double input[], matrix& mat)
{
    /* Takes in a predefined array of doubles and copies over to data array
    of the matrix. I feel that this makes sense, as they're both of the same
    form. Length will need to be validated outside of the operator, as size
    information is not transferred, hence I added getter functions to the class. */
    mat.matrix_data = input;
    return mat;
}

std::ostream& operator<< (std::ostream& output, const matrix& mat)
{
    /* Whilst not perfect (i.e. all rows have the same width), this will display
    a matrix as a series of stacked row vectors. It will, however, insert a comma
    and space after each entry in a row provided it is not in the final column. */
    for(int i{0}; i < mat.rows; i++){
        std::cout << "[";
        for(int j{0}; j < mat.columns; j++){
            int index{i*mat.columns + j};
            std::cout << mat.matrix_data[index];
            if (j < (mat.columns - 1)){
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }
    return output;
}

//int main()
int main()
{
    matrix matrix_a(3,3);
    double a_data[9]{1, 2, 3, 9, 8, 7, 4, 2, 6};
    a_data >> matrix_a; 

    matrix matrix_b(3,3);
    double b_data[9]{5, 5, 4, 1, 2, 3, 6, 9, 8};
    b_data >> matrix_b;

    matrix matrix_c(2,3);
    double c_data[6]{3, 4, 1, 2, 5, 6};
    c_data >> matrix_c;

    std::cout << "Matrix A: " << std::endl;
    std::cout << matrix_a << std::endl;
    std::cout << "Determinant of Matrix A: " << matrix_a.determinant() << std::endl;

    matrix matrix_a_remove{matrix_a.remove(1,1)};
    std::cout << "Matrix A with Row 1 & Column 1 deleted: " << std::endl;
    std::cout << matrix_a_remove << std::endl;

    std::cout << "Matrix B: " << std::endl;
    std::cout << matrix_b << std::endl;
    std::cout << "Determinant of Matrix B: " << matrix_b.determinant() << std::endl;

    std::cout << "Matrix C: " << std::endl;
    std::cout << matrix_c << std::endl;
    std::cout << "Determinant of Matrix C: " << matrix_c.determinant() << std::endl;

    //Calls move constructor (matrix_a + matrix_b is not an object, it is temporary)
    matrix matrix_d{matrix_a + matrix_b};
    //Calls copy constructor (matrix_d IS an object, so it is copied)
    matrix matrix_e{matrix_d};

    std::cout << "Matrix D = A + B: " << std::endl;
    std::cout << matrix_d << std::endl;

    std::cout << "Matrix E = D: " << std::endl;
    std::cout << matrix_e << std::endl;

    //Calls move assignment (again, as matrix_a - matrix_b is temporary, but matrix_d already exists)
    matrix_d = matrix_a - matrix_b;

    std::cout << "D = A - B:" << std::endl;
    std::cout << matrix_d << std::endl;

    //E remains unaffected by the change to D
    std::cout << "Matrix E: " << std::endl;
    std::cout << matrix_e << std::endl;

    matrix matrix_ab{matrix_a * matrix_b};
    //Calls copy assignment (matrix_e already exists and matrix_temp is an object)
    matrix_e = matrix_ab;

    std::cout << "A.B: " << std::endl;
    std::cout << matrix_ab << std::endl;

    std::cout << "Matrix E = A.B: " << std::endl;
    std::cout << matrix_e << std::endl;

    std::cout << "C.B: " << std::endl;
    std::cout << matrix_c * matrix_b << std::endl;

    //Throws the fail message and returns the 1x1 NaN matrix 
    std::cout << "B.C: " << std::endl;
    std::cout << matrix_b * matrix_c << std::endl;
    return 0;
}