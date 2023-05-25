#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#define BYTE_CAPACITY 1024
#define STRUCT_SPACE 36

typedef struct rectangle {
	int object_id;
	double x_low;
	double x_high;
	double y_low;
	double y_high;
}rectangle_t;

typedef struct node {
	int node_id;
	int entries;
	struct rectangle MBR;
	struct node **children;
	struct rectangle **data;
}node_t;

typedef struct level {
	int level_id;
	int nodes;
	double mean_area;
}level_t;

typedef struct queries {
	int results;
	int node_accesses;
}queries_t;

void attributes(rectangle_t *rect, char *line);
void quick_sort(rectangle_t *arr, int first, int last, int type);
void create_leaf_node(node_t *node_arr, rectangle_t **temp, int node_id_counter, int node_capacity);
void create_node(node_t *node_arr, int node_id_counter, int last_node, int nodes, int node_capacity);

int main(int argc, char *argv[]) {
	FILE *infile, *outfile, *queries_file;
	char buf[1024], *dup, ch;
	int i, j, m, k = 0, total_rectangles = 0, number_of_leaves = 0, number_of_slices = 0, node_capacity = 0, last_slice_size = 0, slice_size = 0, node_id_counter = 0, level = 0, nodes = 0, last_node;
	rectangle_t rect, *rect_arr, **slices_arr, **temp, query;
	node_t *node_arr, *root;
	level_t *levels;
	double width = 0.0, height = 0.0, area = 0.0;
	queries_t inter = {0}, ins = {0}, cont = {0};

	if ((infile = fopen(argv[1], "r")) == NULL) {exit(-1);}
	if ((outfile = fopen("rtree.txt", "w")) == NULL) {exit(-1);}

	while (fgets(buf, 1024, infile) != NULL) {
		total_rectangles++;
	}
	
	fseek(infile, 0, SEEK_SET);
	if ((rect_arr = (rectangle_t *) malloc(total_rectangles * sizeof(rectangle_t))) == NULL) {exit(-1);}
	for (i = 0; fgets(buf, 1024, infile) != NULL; i++) {
		dup = strdup(buf);
		attributes(&rect, dup);
		rect_arr[i] = rect;
		free(dup);
	}
	quick_sort(rect_arr, 0, total_rectangles - 1, 1);

	node_capacity = BYTE_CAPACITY / STRUCT_SPACE;
	
	if ((total_rectangles % node_capacity) == 0) {
		number_of_leaves = total_rectangles / node_capacity;
	}
	else {
		number_of_leaves = (total_rectangles / node_capacity) + 1;
	}
	
	if ((int)sqrt(number_of_leaves) < sqrt(number_of_leaves)) {
		number_of_slices = (int)sqrt(number_of_leaves) + 1;
	}
	else {
		number_of_slices = (int)sqrt(number_of_leaves);
	}
	
	nodes = number_of_leaves;
	slice_size = node_capacity * number_of_slices;

	if ((slices_arr = (rectangle_t **) malloc(number_of_slices * sizeof(rectangle_t *))) == NULL) {exit(-1);}
	
	if (total_rectangles <= node_capacity) {
		if ((*slices_arr = (rectangle_t *) malloc(total_rectangles * sizeof(rectangle_t))) == NULL) {exit(-1);}
		memcpy(*slices_arr, rect_arr, total_rectangles * sizeof(rectangle_t));
		quick_sort(*slices_arr, 0, total_rectangles - 1, 2);
	}
	else {
		for (i = 0; i < number_of_slices; i++) {
			if (i == number_of_slices - 2) {
				last_slice_size = total_rectangles - ((number_of_slices - 2) * slice_size);
				if ((slices_arr[i] = (rectangle_t *) malloc(last_slice_size * sizeof(rectangle_t))) == NULL) {exit(-1);}
				memcpy(slices_arr[i], rect_arr + (i * slice_size), last_slice_size * sizeof(rectangle_t));
				quick_sort(slices_arr[i], 0, last_slice_size - 1, 2);
				break;
			}
			if ((slices_arr[i] = (rectangle_t *) malloc(slice_size * sizeof(rectangle_t))) == NULL) {exit(-1);}
			memcpy(slices_arr[i], rect_arr + (i * slice_size), slice_size * sizeof(rectangle_t));		
			quick_sort(slices_arr[i], 0, slice_size - 1, 2);
		}
	}
	//printf("total rectangles: %d\nnumber of slices: %d\nslice size: %d\nlast slice size: %d\nnode capacity: %d\nnumber of leaves: %d\n", total_rectangles, number_of_slices, slice_size, last_slice_size, node_capacity, number_of_leaves);
	m = 0;

	if ((node_arr = (node_t *) malloc((number_of_leaves * sizeof(node_t)))) == NULL) {exit(-1);}
	if ((temp = (rectangle_t **) malloc(node_capacity * sizeof(rectangle_t *))) == NULL) {exit(-1);}

	if (total_rectangles <= node_capacity) {
		for (i = 0; i < total_rectangles; i++) {
			temp[i] = &slices_arr[0][i];
		}
		create_leaf_node(&node_arr[node_id_counter], temp, node_id_counter, total_rectangles);
		width = node_arr[node_id_counter].MBR.x_high - node_arr[node_id_counter].MBR.x_low;
		height = node_arr[node_id_counter].MBR.y_high - node_arr[node_id_counter].MBR.y_low;
		area += width * height;
		node_id_counter++;
		level++;
	}
	else {
		for(i = 0; i < number_of_slices; i++) {
			for (j = 0; j < slice_size && m < total_rectangles; j++, m++) {
				temp[k] = &slices_arr[i][j];
				if (k == node_capacity - 1 || (m == total_rectangles - 1 && k != 0)) {
					create_leaf_node(&node_arr[node_id_counter], temp, node_id_counter, k + 1);
					width = node_arr[node_id_counter].MBR.x_high - node_arr[node_id_counter].MBR.x_low;
					height = node_arr[node_id_counter].MBR.y_high - node_arr[node_id_counter].MBR.y_low;
					area += width * height;
					node_id_counter++;
					k = 0;
					continue;
				}
				k++;
			}
		}
		level++;
	}	
	
	if ((levels = (level_t *) malloc(level * sizeof(level_t))) == NULL) {exit(-1);}
	levels[level - 1].level_id = level;
	levels[level - 1].nodes = nodes;
	levels[level - 1].mean_area = area/(double)nodes;
	
	while (nodes != 1) {
		if (nodes % node_capacity == 0) {
			nodes = nodes / node_capacity;
		}
		else {
			nodes = (nodes / node_capacity) + 1;
		}
		level++;
		area = 0.0;
		last_node = node_id_counter - 1;
		if ((levels = (level_t *) realloc(levels, level * sizeof(level_t))) == NULL) {exit(-1);}
		
		if ((node_arr = (node_t *) realloc(node_arr, (last_node + nodes) * sizeof(node_t))) == NULL) {exit(-1);}
		
		for (i = node_id_counter; i <= last_node + nodes; i++) {
			create_node(&node_arr[i], node_id_counter, last_node, nodes, node_capacity);
			width = node_arr[node_id_counter].MBR.x_high - node_arr[node_id_counter].MBR.x_low;
			height = node_arr[node_id_counter].MBR.y_high - node_arr[node_id_counter].MBR.y_low;
			area += width * height;
			node_id_counter++;
		}
		levels[level - 1].level_id = level;
		levels[level - 1].nodes = nodes;
		levels[level - 1].mean_area = area/(double)nodes;
	}

	root = &node_arr[node_id_counter - 1];

	fprintf(outfile, "%d\n%d\n", root->node_id, level);
	for (i = 0; i < node_id_counter; i++) {
		//printf("%d %2d %lf %lf %lf %lf\n", node_arr[i].node_id, node_arr[i].entries, node_arr[i].MBR.x_low, node_arr[i].MBR.x_high, node_arr[i].MBR.y_low, node_arr[i].MBR.y_high);
		fprintf(outfile, "%4d, %2d", node_arr[i].node_id, node_arr[i].entries);
		for (j = 0; j < node_arr[i].entries; j++) {
			if (i < number_of_leaves) {
				fprintf(outfile, ", (%6d, %.6lf, %.6lf, %.6lf, %.6lf)", node_arr[i].data[j]->object_id, node_arr[i].data[j]->x_low, node_arr[i].data[j]->x_high, node_arr[i].data[j]->y_low, node_arr[i].data[j]->y_high);
			}
			else {
				fprintf(outfile, ", (%6d, %.6lf, %.6lf, %.6lf, %.6lf)", node_arr[i].children[j]->node_id, node_arr[i].children[j]->MBR.x_low, node_arr[i].children[j]->MBR.x_high, node_arr[i].children[j]->MBR.y_low, node_arr[i].children[j]->MBR.y_high);
			}
		}
		fprintf(outfile, "\n");
	}
		
	printf("Level\tNodes\tMean area\n");
	for (i = 0; i < level; i++) {
		printf("%d\t%d\t%.6lf\n", levels[i].level_id, levels[i].nodes, levels[i].mean_area);
	}
	
	return 0;
}

void create_node(node_t *node_arr, int node_id_counter, int last_node, int nodes, int node_capacity) {
	int i;
	static int j = 0;

	node_arr->node_id = node_id_counter;
	node_arr->MBR.x_low = node_arr[-node_id_counter + j].MBR.x_low;
	node_arr->MBR.x_high = node_arr[-node_id_counter + j].MBR.x_high;
	node_arr->MBR.y_low = node_arr[-node_id_counter + j].MBR.y_low;
	node_arr->MBR.y_high = node_arr[-node_id_counter + j].MBR.y_high;
	node_arr->data = NULL;
	
	if ((node_arr->children = (node_t **) malloc(node_capacity * sizeof(node_t *))) == NULL) {exit(-1);}
	for (i = 0, j; i < node_capacity && j <= last_node; i++, j++) {
		if (node_arr[-node_id_counter + j].MBR.x_low < node_arr->MBR.x_low)
			node_arr->MBR.x_low = node_arr[-node_id_counter + j].MBR.x_low;
		if (node_arr[-node_id_counter + j].MBR.x_high > node_arr->MBR.x_high)
			node_arr->MBR.x_high = node_arr[-node_id_counter + j].MBR.x_high;
		if (node_arr[-node_id_counter + j].MBR.y_low < node_arr->MBR.y_low)
			node_arr->MBR.y_low = node_arr[-node_id_counter + j].MBR.y_low;
		if (node_arr[-node_id_counter + j].MBR.y_high > node_arr->MBR.y_high)
			node_arr->MBR.y_high = node_arr[-node_id_counter + j].MBR.y_high;
		node_arr->children[i] = &node_arr[-node_id_counter + j];
	}
	
	node_arr->entries = i;
	if (i < node_capacity) {
		if ((node_arr->children = (node_t **) realloc(node_arr->children, i * sizeof(node_t *))) == NULL) {exit(-1);}
	}
}

void create_leaf_node(node_t *node_arr, rectangle_t **temp, int node_id_counter, int node_capacity) {
	int i, j;
	
	node_arr->node_id = node_id_counter;
	node_arr->MBR.x_low = temp[0]->x_low;
	node_arr->MBR.x_high = temp[0]->x_high;
	node_arr->MBR.y_low = temp[0]->y_low;
	node_arr->MBR.y_high = temp[0]->y_high;
	node_arr->entries = node_capacity;
	node_arr->children = NULL;
	
	for (i = 0; i < node_capacity; i++) {
		if (temp[i]->x_low < node_arr->MBR.x_low)
			node_arr->MBR.x_low = temp[i]->x_low;
		if (temp[i]->x_high > node_arr->MBR.x_high)
			node_arr->MBR.x_high = temp[i]->x_high;
		if (temp[i]->y_low < node_arr->MBR.y_low)
			node_arr->MBR.y_low = temp[i]->y_low;
		if (temp[i]->y_high > node_arr->MBR.y_high)
			node_arr->MBR.y_high = temp[i]->y_high;
	}
	
	if ((node_arr->data = (rectangle_t **) malloc(node_capacity * sizeof(rectangle_t *))) == NULL) {exit(-1);}
	memcpy(node_arr->data, temp, node_capacity * sizeof(rectangle_t *));
}

void attributes(rectangle_t *rect, char *line) {
	int i;
	char *token;

	for (i = 0, token = strtok(line, "\t"); token != NULL; i++, token = strtok(NULL, "\t")) {
		switch (i) {
			case 0:
				rect->object_id = atoi(token);
				break;
			case 1:
				rect->x_low = atof(token);
				break;
			case 2:
				rect->x_high = atof(token);
				break;
			case 3:
				rect->y_low = atof(token);
				break;
			case 4:
				rect->y_high = atof(token);
				break;
			default:
				break;
		}
	}
}

void quick_sort(rectangle_t *arr, int first, int last, int type) {
	int i, j, pivot;
	rectangle_t temp;
	
	switch (type) {
		case 1:
			if (first < last) {
				pivot = first;
				i = first;
				j = last;
				while (i < j) {
					while (arr[i].x_low <= arr[pivot].x_low && i < last) {
						i++;
					}
					while (arr[j].x_low > arr[pivot].x_low) {
						j--;
					}
					if (i < j) {
						temp = arr[i];
						arr[i] = arr[j];
						arr[j] = temp;
					}
				}
				temp = arr[pivot];
				arr[pivot] = arr[j];
				arr[j] = temp;
				quick_sort(arr, first, j - 1, type);
				quick_sort(arr, j + 1, last, type);
			}
			break;
		case 2:
			if (first < last) {
				pivot = first;
				i = first;
				j = last;
				while (i < j) {
					while (arr[i].y_low <= arr[pivot].y_low && i < last) {
						i++;
					}
					while (arr[j].y_low > arr[pivot].y_low) {
						j--;
					}
					if (i < j) {
						temp = arr[i];
						arr[i] = arr[j];
						arr[j] = temp;
					}
				}
				temp = arr[pivot];
				arr[pivot] = arr[j];
				arr[j] = temp;
				quick_sort(arr, first, j - 1, type);
				quick_sort(arr, j + 1, last, type);
			}
			break;
		default:
			break;
	}
}
