#ifndef NEW_H
#define NEW_H

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

typedef double (*D2D)(double);
typedef struct
{
    D2D af, d_af;
    double *arg;
} AF;
double ReLU(double x)
{
    return x > 0 ? x : 0;
}
double d_ReLU(double x)
{
    return x > 0 ? 1 : 0;
}
#define f_ReLU       \
    {                \
        ReLU, d_ReLU \
    }
double sigmoid(double x)
{
    return 1 / (1 + exp(-x));
}
double d_sigmoid(double x)
{
    return sigmoid(x) * (1 - sigmoid(x));
}
#define f_sigmoid          \
    {                      \
        sigmoid, d_sigmoid \
    }

double d_tanh(double x)
{
    return 1 - tanh(x) * tanh(x);
}
#define f_tanh       \
    {                \
        tanh, d_tanh \
    }

typedef double (*DSDSUL2D)(double *, double *, unsigned long);
typedef double (*DDUL2D)(double, double, unsigned long);
typedef struct
{
    DSDSUL2D ls;
    DDUL2D d_LS;
} LF;
double MSE(double *array1, double *array2, unsigned long size)
{
    double t = 0;
    for (unsigned long i = 0; i < size; ++i)
        t += (array1[i] - array2[i]) * (array1[i] - array2[i]);
    return t / size;
}
double d_MSE(double num1, double num2, unsigned long size)
{
    return 2 * (num1 - num2) / size;
}
#define f_MSE      \
    {              \
        MSE, d_MSE \
    }
typedef struct
{
    unsigned long size;
    double *weight, *bais, *error, *output;
    AF activition_function;
} FullConnectLayer;
typedef struct
{
    unsigned long size;
    double *true_output;
    FullConnectLayer *layers;
    LF lossing_function;
} DNN;
typedef struct
{
    unsigned long size, in_size, out_size;
    double **in_data, **out_data;
} Data;
DNN *CreateDNN(unsigned long size_of_layer, unsigned long *size_of_every_layer, AF *activition_funtion_of_every_layer, LF lossing_function)
{
    DNN *net = malloc(sizeof(DNN));
    srand(net);
    net->lossing_function = lossing_function;
    net->size = size_of_layer;
    net->layers = malloc(size_of_layer * sizeof(FullConnectLayer));
    net->layers[0].size = size_of_every_layer[0];
    for (unsigned long i = 1; i < net->size; ++i)
    {
        net->layers[i].size = size_of_every_layer[i];
        net->layers[i].activition_function = activition_funtion_of_every_layer[i];
        net->layers[i].bais = malloc(sizeof(double) * net->layers[i].size);
        net->layers[i].error = malloc(sizeof(double) * net->layers[i].size);
        net->layers[i].output = malloc(sizeof(double) * net->layers[i].size);
        net->layers[i].weight = malloc(sizeof(double *) * net->layers[i].size * net->layers[i - 1].size);
        for (unsigned long j = 0; j < net->layers[i].size; ++j)
        {
            net->layers[i].bais[j] = 1.0 / (rand()+1);
            net->layers[i].error[j] = net->layers[i].output[j] = 0;
            for (unsigned long k = 0; k < net->layers[i - 1].size; ++k)
                net->layers[i].weight[j * net->layers[i].size + k] = 1.0 / (rand() + 1);
        }
    }
    return net;
}
Data *CreateData(unsigned long size, unsigned long in_size, unsigned long out_size)
{
    Data *data = malloc(sizeof(Data));
    data->size = size;
    data->in_size = in_size;
    data->out_size = out_size;
    data->in_data = malloc(sizeof(double *) * data->size);
    data->out_data = malloc(sizeof(double *) * data->size);
    for (unsigned long i = 0; i < data->size; ++i)
    {
        data->in_data[i] = malloc(sizeof(double) * data->in_size);
        data->out_data[i] = malloc(sizeof(double) * data->out_size);
    }
    return data;
}
void LoadDataIn(DNN *net, double *input, double *true_output)
{
    net->layers[0].output = input;
    net->true_output = true_output;
}
double GetError(DNN *net)
{
    return net->lossing_function.ls(net->layers[net->size - 1].output, net->true_output, net->layers[net->size - 1].size);
}
void RunOnce(DNN *net)
{
    for (unsigned long i = 1; i < net->size; ++i)
        for (unsigned j = 0; j < net->layers[i].size; ++j)
        {
            double t = 0;
            for (unsigned long k = 0; k < net->layers[i - 1].size; ++k)
                t += net->layers[i - 1].output[k] * net->layers[i].weight[j * net->layers[i].size + k];
            net->layers[i].output[j] = net->layers[i].activition_function.af(t + net->layers[i].bais[j]);
        }
}
void FreeDNN(DNN *net)
{
    for (unsigned long i = 1; i < net->size; ++i)
    {
        free(net->layers[i].bais);
        free(net->layers[i].output);
        free(net->layers[i].error);
        free(net->layers[i].weight);
    }
    free(net);
}


void GD_Train(DNN *net, unsigned long epoch, double learning_rate, Data *data, double E)
{
    for (unsigned long t = 0; t < epoch; ++t)
    {
        for (unsigned long r = 0; r < data->size; ++r)
        {
            LoadDataIn(net, data->in_data[r], data->out_data[r]);
            RunOnce(net);
            for (unsigned long i = 0; i < net->layers[net->size - 1].size; ++i)
            {
                net->layers[net->size - 1].error[i] = net->layers[net->size - 1].activition_function.d_af(net->layers[net->size - 1].output[i]) * net->lossing_function.d_LS(net->layers[net->size - 1].output[i], net->true_output[i], net->layers[i].size);
                net->layers[net->size - 1].bais[i] -= learning_rate * net->layers[net->size - 1].error[i];
                for (unsigned long j = 0; j < net->layers[net->size - 2].size; ++j)
                    net->layers[net->size - 1].weight[i * net->layers[net->size - 1].size + j] -= learning_rate * net->layers[net->size - 1].error[i] * net->layers[net->size - 1 - 1].output[j];
            }
            for (unsigned long i = net->size - 2; i > 0; --i)
                for (unsigned long j = 0; j < net->layers[i].size; ++j)
                {
                    net->layers[i].error[j] = 0;
                    for (unsigned long k = 0; k < net->layers[i + 1].size; ++k)
                        net->layers[i].error[j] += net->layers[i + 1].weight[k * net->layers[i + 1].size + j] * net->layers[i + 1].error[k] * net->layers[i + 1].activition_function.d_af(net->layers[i].output[j]);
                    net->layers[i].bais[j] -= learning_rate * net->layers[i].error[j];
                    for (unsigned long k = 0; k < net->layers[i - 1].size; ++k)
                        net->layers[i].weight[j * net->layers[i].size + k] -= learning_rate * net->layers[i].error[j] * net->layers[i - 1].output[k];
                }
        }
        if (GetError(net) < E)
            break;
    }
}
void SGD_Train(DNN *net, unsigned long epoch, unsigned long batch_size, double learning_rate, Data *data, double E)
{
    for (unsigned long t = 0; t < epoch; ++t)
    {
        for (unsigned long r = 0; r < data->size / batch_size; ++r)
        {
            unsigned long rr = rand() % data->size;
            LoadDataIn(net, data->in_data[rr], data->out_data[rr]);
            RunOnce(net);
            for (unsigned long i = 0; i < net->layers[net->size - 1].size; ++i)
            {
                net->layers[net->size - 1].error[i] = net->layers[net->size - 1].activition_function.d_af(net->layers[net->size - 1].output[i]) * net->lossing_function.d_LS(+net->layers[net->size - 1].output[i], net->true_output[i], net->layers[i].size);
                net->layers[net->size - 1].bais[i] -= learning_rate * net->layers[net->size - 1].error[i];
                for (unsigned long j = 0; j < net->layers[net->size - 2].size; ++j)
                    net->layers[net->size - 1].weight[i * net->layers[net->size - 1].size + j] -= learning_rate * net->layers[net->size - 1].error[i] * net->layers[net->size - 1 - 1].output[j];
            }
            for (unsigned long i = net->size - 2; i > 0; --i)
                for (unsigned long j = 0; j < net->layers[i].size; ++j)
                {
                    net->layers[i].error[j] = 0;
                    for (unsigned long k = 0; k < net->layers[i + 1].size; ++k)
                        net->layers[i].error[j] += net->layers[i + 1].weight[k * net->layers[i + 1].size + j] * net->layers[i + 1].error[k] * net->layers[i + 1].activition_function.d_af(net->layers[i].output[j]);
                    net->layers[i].bais[j] -= learning_rate * net->layers[i].error[j];
                    for (unsigned long k = 0; k < net->layers[i - 1].size; ++k)
                        net->layers[i].weight[j * net->layers[i].size + k] -= learning_rate * net->layers[i].error[j] * net->layers[i - 1].output[k];
                }
        }
        if (GetError(net) < E)
            break;
    }
}
void SGDM_Train(DNN *net, unsigned long epoch, unsigned long batch_size, double A, double learning_rate, Data *data, double E)
{
    double ***v_w = malloc(net->size * sizeof(double **)), **v_b = malloc(net->size * sizeof(double *));
    for (unsigned long i = 1; i < net->size; ++i)
    {
        v_w[i] = malloc(net->layers[i].size * sizeof(double *));
        v_b[i] = malloc(net->layers[i].size * sizeof(double));
        for (unsigned long j = 0; j < net->layers[i].size; ++j)
        {
            v_w[i][j] = malloc(net->layers[i - 1].size * sizeof(double));
            v_b[i][j] = 0;
            for (unsigned long k = 0; k < net->layers[i - 1].size; ++k)
                v_w[i][j][k] = 0;
        }
    }
    for (unsigned long t = 0; t < epoch; ++t)
    {
        for (unsigned long r = 0; r < data->size / batch_size; ++r)
        {
            unsigned long rr = rand() % data->size;
            LoadDataIn(net, data->in_data[rr], data->out_data[rr]);
            RunOnce(net);
            for (unsigned long i = 0; i < net->layers[net->size - 1].size; ++i)
            {
                net->layers[net->size - 1].error[i] = net->layers[net->size - 1].activition_function.d_af(net->layers[net->size - 1].output[i]) * net->lossing_function.d_LS(+net->layers[net->size - 1].output[i], net->true_output[i], net->layers[i].size);
                v_b[net->size - 1][i] = v_b[net->size - 1][i] * A + learning_rate * net->layers[net->size - 1].error[i];
                net->layers[net->size - 1].bais[i] -= v_b[net->size - 1][i];
                for (unsigned long j = 0; j < net->layers[net->size - 2].size; ++j)
                {
                    v_w[net->size - 1][i][j] = v_w[net->size - 1][i][j] * A + learning_rate * net->layers[net->size - 1].error[i] * net->layers[net->size - 1 - 1].output[j];
                    net->layers[net->size - 1].weight[i * net->layers[net->size - 1].size + j] -= v_w[net->size - 1][i][j];
                }
            }
            for (unsigned long i = net->size - 2; i > 0; --i)
                for (unsigned long j = 0; j < net->layers[i].size; ++j)
                {
                    net->layers[i].error[j] = 0;
                    for (unsigned long k = 0; k < net->layers[i + 1].size; ++k)
                        net->layers[i].error[j] += net->layers[i + 1].weight[k * net->layers[i + 1].size + j] * net->layers[i + 1].error[k] * net->layers[i + 1].activition_function.d_af(net->layers[i].output[j]);
                    v_b[i][j] = v_b[i][j] * A + learning_rate * net->layers[i].error[j];
                    net->layers[i].bais[j] -= v_b[i][j];
                    for (unsigned long k = 0; k < net->layers[i - 1].size; ++k)
                    {
                        v_w[i][j][k] = v_w[i][j][k] * A + learning_rate * net->layers[i].error[j] * net->layers[i - 1].output[k];
                        net->layers[i].weight[j * net->layers[i].size + k] -= v_w[i][j][k];
                    }
                }
        }
        for (unsigned long i = 1; i < net->size; ++i)
            for (unsigned long j = 0; j < net->layers[i].size; ++j)
            {
                v_b[i][j] = 0;
                for (unsigned long k = 0; k < net->layers[i - 1].size; ++k)
                    v_w[i][j][k] = 0;
            }
        if (GetError(net) < E)
            break;
    }
    for (unsigned long i = 1; i < net->size; ++i)
    {
        free(v_b[i]);
        for (unsigned long j = 0; j < net->layers[i].size; ++j)
            free(v_w[i][j]);
        free(v_w[i]);
    }
    free(v_b);
    free(v_w);
}
void NAG_Train(DNN *net, unsigned long epoch, unsigned long batch_size, double A, double learning_rate, Data *data, double E){

    double ***v_w = malloc(net->size * sizeof(double **)), **v_b = malloc(net->size * sizeof(double *));
    for (unsigned long i = 1; i < net->size; ++i)
    {
        v_w[i] = malloc(net->layers[i].size * sizeof(double *));
        v_b[i] = malloc(net->layers[i].size * sizeof(double));
        for (unsigned long j = 0; j < net->layers[i].size; ++j)
        {
            v_w[i][j] = malloc(net->layers[i - 1].size * sizeof(double));
            v_b[i][j] = 0;
            for (unsigned long k = 0; k < net->layers[i - 1].size; ++k)
                v_w[i][j][k] = 0;
        }
    }
    for (unsigned long t = 0; t < epoch; ++t)
    {
        for (unsigned long r = 0; r < data->size / batch_size; ++r)
        {
            unsigned long rr = rand() % data->size;
            LoadDataIn(net, data->in_data[rr], data->out_data[rr]);
            RunOnce(net);
            for (unsigned long i = net->size - 1; i > 0; --i)
                for (unsigned long j = 0; j < net->layers[i].size; ++j)
                {
                    net->layers[i].bais[j] -= v_b[i][j];
                    for (unsigned long k = 0; k < net->layers[i - 1].size; ++k)
                        net->layers[i].weight[j * net->layers[i].size + k] -= v_w[i][j][k];
                }
            for (unsigned long i = 0; i < net->layers[net->size - 1].size; ++i)
                net->layers[net->size - 1].error[i] = net->layers[net->size - 1].activition_function.d_af(net->layers[net->size - 1].output[i]) * net->lossing_function.d_LS(+net->layers[net->size - 1].output[i], net->true_output[i], net->layers[i].size);
            for (unsigned long i = net->size - 2; i > 0; --i)
                for (unsigned long j = 0; j < net->layers[i].size; ++j)
                {
                    net->layers[i].error[j] = 0;
                    for (unsigned long k = 0; k < net->layers[i + 1].size; ++k)
                        net->layers[i].error[j] += net->layers[i + 1].weight[k * net->layers[i + 1].size + j] * net->layers[i + 1].error[k] * net->layers[i + 1].activition_function.d_af(net->layers[i].output[j]);
                }
            for (unsigned long i = net->size - 1; i > 0; --i)
                for (unsigned long j = 0; j < net->layers[i].size; ++j)
                {
                    net->layers[i].bais[j] += v_b[i][j];
                    for (unsigned long k = 0; k < net->layers[i - 1].size; ++k)
                        net->layers[i].weight[j * net->layers[i].size + k] += v_w[i][j][k];
                }
            for (unsigned long i = net->size - 1; i > 0; --i)
                for (unsigned long j = 0; j < net->layers[i].size; ++j)
                {
                    v_b[i][j] = v_b[i][j] * A + learning_rate * net->layers[i].error[j];
                    net->layers[i].bais[j] -= v_b[i][j];
                    for (unsigned long k = 0; k < net->layers[i - 1].size; ++k)
                    {
                        v_w[i][j][k] = v_w[i][j][k] * A + learning_rate * net->layers[i].error[j] * net->layers[i - 1].output[k];
                        net->layers[i].weight[j * net->layers[i].size + k] -= v_w[i][j][k];
                    }
                }
        }
        for (unsigned long i = 1; i < net->size; ++i)
            for (unsigned long j = 0; j < net->layers[i].size; ++j)
            {
                v_b[i][j] = 0;
                for (unsigned long k = 0; k < net->layers[i - 1].size; ++k)
                    v_w[i][j][k] = 0;
            }
        if (GetError(net) < E)
            break;
    }
    for (unsigned long i = 1; i < net->size; ++i)
    {
        free(v_b[i]);
        for (unsigned long j = 0; j < net->layers[i].size; ++j)
            free(v_w[i][j]);
        free(v_w[i]);
    }
    free(v_b);
    free(v_w);
}
#endif