#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hcq.h"
#define INPUT_BUFFER_SIZE 256

/*
 * Return a pointer to the struct student with name stu_name
 * or NULL if no student with this name exists in the stu_list
 */
Student *find_student(Student *stu_list, char *student_name) {
    Student *current_student = stu_list;
    while (current_student != NULL) {
        if (strcmp(current_student->name, student_name) == 0) {
            return current_student;
        }
        current_student = current_student->next_overall;
    }
    return NULL;
}

/*   Return a pointer to the ta with name ta_name or NULL
 *   if no such TA exists in ta_list. 
 */
Ta *find_ta(Ta *ta_list, char *ta_name) {
    Ta *curr_ta = ta_list;
    while(curr_ta != NULL) {
        if (strcmp(curr_ta->name, ta_name) == 0) {
            return curr_ta;
        }
        curr_ta = curr_ta->next;
    }
    return NULL;
}

/*  Return a pointer to the course with this code in the course list
 *  or NULL if there is no course in the list with this code.
 */
Course *find_course(Course *courses, int num_courses, char *course_code) {
    for (int i = 0; i < num_courses; i++) {
        if ( strcmp( courses[i].code, course_code ) == 0 ) {
            return &(courses[i]);
        }
    }
    return NULL;
}

/* Return a pointer to the student in front of student student_name in the
 * queue for the given course, or the overall queue if course_code is NULL.
 * Return NULL if student_name is the first in the overall or the course queue.
 * Return the last student in the specified queue if student_name is NULL.
 * Return NULL if the overall queue or course queue is empty.
 * 
 * Preconditions: course_code is a valid course in the course list or NULL
 *                student_name is a valid student in the student list or NULL.
 *                student_name is a student in course_code
 */
Student *student_before(Student *stu_list, char *student_name, char *course_code) {
    Student *curr_student = stu_list;
    if (curr_student == NULL) {
        return NULL;
    }
    
    // At the end if this if statement, curr_student will hold the pointer
    // to the student we're looking for
    if (course_code == NULL && student_name == NULL) {
        // Iterate to the last student in the overall queue
        while (curr_student->next_overall != NULL) {
            curr_student = curr_student->next_overall;
        }

    } else if (course_code == NULL) {
        // Iterate to the student before student_name in the overall queue
        if (strcmp(curr_student->name, student_name) == 0) {
            return NULL;
        }
        // By our preconditions we can assume that student_name exists in queue
        Student *next_student = curr_student->next_overall;
        while (strcmp(next_student->name, student_name) != 0) {
            curr_student = next_student;
            next_student = next_student->next_overall;
        }

    } else if (student_name == NULL) {
        // Iterate to the last student in the course queue, if there is one

        // First iterate to the first student in the course queue
        while (curr_student != NULL && strcmp(curr_student->course->code, course_code) != 0) {
            curr_student = curr_student->next_overall;
        }

        // Finally iterate to the last student in course queue
        while (curr_student->next_course != NULL) {
            curr_student = curr_student->next_course;
        }

    } else {
        // Iterate to the student before student_name in the course queue

        // Same as above, find first student in course queue
        while (curr_student != NULL && strcmp(curr_student->course->code, course_code) != 0) {
            curr_student = curr_student->next_overall;
        }

        // Iterate to student before student_name
        if (strcmp(curr_student->name, student_name) == 0) {
            // curr_student is at the front of the queue
            return NULL;
        }
        Student *next_student = curr_student->next_course;
        while (strcmp(next_student->name, student_name) != 0) {
            curr_student = next_student;
            next_student = next_student->next_overall;
        }
    }

    return curr_student;
}

/* Add a student to the queue with student_name and a question about course_code.
 * if a student with this name already has a question in the queue (for any
   course), return 1 and do not create the student.
 * If course_code does not exist in the list, return 2 and do not create
 * the student struct.
 * For the purposes of this assignment, don't check anything about the 
 * uniqueness of the name. 
 */
int add_student(Student **stu_list_ptr, char *student_name, char *course_code,
    Course *course_array, int num_courses) {
    if (find_student(*stu_list_ptr, student_name) != NULL) {
        return 1;
    }

    Course *course_to_join = find_course(course_array, num_courses, course_code);
    if (course_to_join == NULL) {
        return 2;
    }

    // Construct student
    Student *student_to_add = malloc(sizeof(Student));
    if (student_to_add == NULL) {
        perror("malloc for add_student struct");
        exit(1);
    }

    student_to_add->name = malloc( (strlen(student_name) + 1) * sizeof(char) );
    if (student_to_add->name == NULL) {
        perror("malloc for add_student name");
        exit(1);
    }
    strcpy(student_to_add->name, student_name);

    student_to_add->arrival_time = malloc(sizeof(time_t));
    if (student_to_add->arrival_time == NULL) {
        perror("malloc for add_student arrival_time");
        exit(1);
    }
    *(student_to_add->arrival_time) = time(NULL);

    student_to_add->course = course_to_join;
    student_to_add->next_overall = NULL;
    student_to_add->next_course = NULL;


    // Add student as last in overall and course queue
    Student *last_overall_student = student_before(*stu_list_ptr, NULL, NULL);
    if (last_overall_student == NULL) {
        // Then the list is empty and they go at the front of the queues
        *stu_list_ptr = student_to_add;
        course_to_join->head = student_to_add;
        course_to_join->tail = student_to_add;
    } else {
        last_overall_student->next_overall = student_to_add;

        // Add student to tail and head if necessary
        if (course_to_join->head == NULL) {
            course_to_join->head = student_to_add;
        }
        if (course_to_join->tail != NULL) {
            course_to_join->tail->next_course = student_to_add;
        }
        course_to_join->tail = student_to_add;
    }
    
    return 0;
}

/* Student student_name has given up waiting and left the help centre
 * before being called by a Ta. Record the appropriate statistics, remove
 * the student from the queues and clean up any no-longer-needed memory.
 *
 * If there is no student by this name in the stu_list, return 1.
 */
int give_up_waiting(Student **stu_list_ptr, char *student_name) {
    Student *bailer = find_student(*stu_list_ptr, student_name);
    if (bailer == NULL) {
        return 1;
    }
    Course *course_to_bail = bailer->course;

    course_to_bail->bailed = course_to_bail->bailed + 1;

    // Update total wait time
    time_t curr_time = time(NULL);
    double time_difference = difftime(curr_time, *(bailer->arrival_time));
    course_to_bail->wait_time += time_difference;

    // Remove the student from the queues
    Student *stu_overall_before_bailer = student_before(*stu_list_ptr, student_name, NULL);
    Student *stu_course_before_bailer = student_before(*stu_list_ptr, student_name, bailer->course->code);

    if (stu_overall_before_bailer == NULL) {
        // Then bailer is at the head of both queues
        *stu_list_ptr = bailer->next_overall;
        course_to_bail->head = bailer->next_course;

        if (strcmp(course_to_bail->tail->name, bailer->name) == 0) {
            // Bailer is at the head and tail of the course queue
            course_to_bail->tail = NULL;
        }
    } else {
        // Bailer is not at the head of the overall queue
        stu_overall_before_bailer->next_overall = bailer->next_overall;

        if (stu_course_before_bailer == NULL) {
            // Bailer is at the head of the course queue, possibly also the tail
            course_to_bail->head = bailer->next_course;
        } else {
            // Bailer is not the head, so student in front needs to be linked
            // to the person behind bailer
            stu_course_before_bailer->next_course = bailer->next_course;
        }

        if (strcmp(course_to_bail->tail->name, bailer->name) == 0) {
            // Bailer is at the tail of the course queue

            // stu_course_before_bailer is NULL if bailer is the head of
            // the course queue, and else is the course student before bailer.
            // In either case this correctly sets the tail.
            course_to_bail->tail = stu_course_before_bailer;
        }
    }

    free(bailer->name);
    free(bailer->arrival_time);
    free(bailer);
    
    return 0;
}

/* Create and prepend Ta with ta_name to the head of ta_list. 
 * For the purposes of this assignment, assume that ta_name is unique
 * to the help centre and don't check it.
 */
void add_ta(Ta **ta_list_ptr, char *ta_name) {
    // first create the new Ta struct and populate
    Ta *new_ta = malloc(sizeof(Ta));
    if (new_ta == NULL) {
       perror("malloc for TA");
       exit(1);
    }
    new_ta->name = malloc(strlen(ta_name)+1);
    if (new_ta->name  == NULL) {
       perror("malloc for TA name");
       exit(1);
    }
    strcpy(new_ta->name, ta_name);
    new_ta->current_student = NULL;

    // insert into front of list
    new_ta->next = *ta_list_ptr;
    *ta_list_ptr = new_ta;
}

/* The TA ta is done with their current student. 
 * Calculate the stats (the times etc.) and then 
 * free the memory for the student. 
 * If the TA has no current student, do nothing.
 */
void release_current_student(Ta *ta) {
    if (ta->current_student == NULL) {
        return;
    }
    Student *student_to_release = ta->current_student;
    Course *course = student_to_release->course;

    // Update course stats
    course->helped = course->helped + 1;

    time_t curr_time = time(NULL);
    double time_difference = difftime(curr_time, *(student_to_release->arrival_time));
    course->help_time += time_difference;

    free(student_to_release->name);
    free(student_to_release->arrival_time);
    free(student_to_release);

    ta->current_student = NULL;
}

/* Remove this Ta from the ta_list and free the associated memory with
 * both the Ta we are removing and the current student (if any).
 * Return 0 on success or 1 if this ta_name is not found in the list
 */
int remove_ta(Ta **ta_list_ptr, char *ta_name) {
    Ta *head = *ta_list_ptr;
    if (head == NULL) {
        return 1;
    } else if (strcmp(head->name, ta_name) == 0) {
        // TA is at the head so special case
        *ta_list_ptr = head->next;
        release_current_student(head);
        // memory for the student has been freed. Now free memory for the TA.
        free(head->name);
        free(head);
        return 0;
    }
    while (head->next != NULL) {
        if (strcmp(head->next->name, ta_name) == 0) {
            Ta *ta_tofree = head->next;
            //  We have found the ta to remove, but before we do that 
            //  we need to finish with the student and free the student.
            //  You need to complete this helper function
            release_current_student(ta_tofree);

            head->next = head->next->next;
            // memory for the student has been freed. Now free memory for the TA.
            free(ta_tofree->name);
            free(ta_tofree);
            return 0;
        }
        head = head->next;
    }
    // if we reach here, the ta_name was not in the list
    return 1;
}

/* Student student_to_help is about to start being helped by a TA,
 * this function updates the wait_time statistics for the student's course.
 */
void update_waiting_statistics(Student *student_to_help) {
    Course *student_course = student_to_help->course;
    time_t curr_time = time(NULL);
    double time_difference = difftime(curr_time, *(student_to_help->arrival_time));
    student_course->wait_time += time_difference;

    *(student_to_help->arrival_time) = curr_time;
}

/* TA ta_name is finished with the student they are currently helping (if any)
 * and are assigned to the next student in the full queue. 
 * If the queue is empty, then TA ta_name simply finishes with the student 
 * they are currently helping, records appropriate statistics, 
 * and sets current_student for this TA to NULL.
 * If ta_name is not in ta_list, return 1 and do nothing.
 */
int take_next_overall(char *ta_name, Ta *ta_list, Student **stu_list_ptr) {
    Ta *ta_finished = find_ta(ta_list, ta_name);
    if (ta_finished == NULL) {
        return 1;
    }

    // Time helped and num helped stats updated in release_current_student
    release_current_student(ta_finished);

    Student *student_to_help = *stu_list_ptr;
    if (student_to_help != NULL) {
        ta_finished->current_student = student_to_help;

        // Set new heads of overall and course queues, and tail if necessary
        *stu_list_ptr = student_to_help->next_overall;
        student_to_help->course->head = student_to_help->next_course;
        if (strcmp(student_to_help->name, student_to_help->course->tail->name) == 0) {
            // Then there are to be no students left in this course queue
            student_to_help->course->tail = NULL;
        }

        update_waiting_statistics(student_to_help);

        student_to_help->next_overall = NULL;
        student_to_help->next_course = NULL;
    }
    return 0;
}

/* TA ta_name is finished with the student they are currently helping (if any)
 * and are assigned to the next student in the course with this course_code. 
 * If no student is waiting for this course, then TA ta_name simply finishes 
 * with the student they are currently helping, records appropriate statistics,
 * and sets current_student for this TA to NULL.
 * If ta_name is not in ta_list, return 1 and do nothing.
 * If course is invalid return 2, but finish with any current student. 
 */
int take_next_course(char *ta_name, Ta *ta_list, Student **stu_list_ptr, char *course_code, Course *courses, int num_courses) {
    Ta *ta_finished = find_ta(ta_list, ta_name);
    if (ta_finished == NULL) {
        return 1;
    }
    Course *ta_course = find_course(courses, num_courses, course_code);
    if (ta_course == NULL) {
        return 2;
    }

    // Time helped and num helped stats updated in release_current_student
    release_current_student(ta_finished);

    // Find first student from course
    Student *student_to_help = *stu_list_ptr;
    while (student_to_help != NULL && strcmp(student_to_help->course->code, course_code) != 0) {
        student_to_help = student_to_help->next_overall;
    }

    if (student_to_help != NULL) {
        ta_finished->current_student = student_to_help;

        Student *stu_before = student_before(*stu_list_ptr, student_to_help->name, NULL);

        // Remove from overall queue
        if (stu_before == NULL) {
            // If at head of overall queue
            *stu_list_ptr = student_to_help->next_overall;
        } else {
            // If not at head
            stu_before->next_overall = student_to_help->next_overall;
        }

        // Set head either way, set tail if necessary
        ta_course->head = student_to_help->next_course;
        if (strcmp(student_to_help->name, ta_course->tail->name) == 0) {
            ta_course->tail = NULL;
        }

        update_waiting_statistics(student_to_help);

        student_to_help->next_overall = NULL;
        student_to_help->next_course = NULL;
    }

    return 0;
}

/* For each course (in the same order as in the config file), print
 * the <course code>: <number of students waiting> "in queue\n" followed by
 * one line per student waiting with the format "\t%s\n" (tab name newline)
 * Uncomment and use the printf statements below. Only change the variable
 * names.
 */
void print_all_queues(Student *stu_list, Course *courses, int num_courses) {
    for (int i = 0; i < num_courses; i++) {
        Course *curr_course = &courses[i];

        // Count students in curr_course
        int num_students = 0;
        Student *current_student = curr_course->head;
        while(current_student != NULL) {
            num_students++;
            current_student = current_student->next_course;
        }

        // Print course information
        printf("%s: %d in queue\n", curr_course->code, num_students);

        Student *curr_student = curr_course->head;
        while (curr_student != NULL) {
            printf("\t%s\n", curr_student->name);
            curr_student = curr_student->next_course;
        }
    }
}

/*
 * Print to stdout, a list of each TA, who they are serving at from what course
 * Uncomment and use the printf statements 
 */
void print_currently_serving(Ta *ta_list) {
    if (ta_list == NULL) {
        printf("No TAs are in the help centre.\n");

    } else {
        Ta *curr_ta = ta_list;
        while (curr_ta != NULL) {
            if (curr_ta->current_student == NULL) {
                printf("TA: %s has no student\n", curr_ta->name);
            } else {
                char *stu_name = curr_ta->current_student->name;
                char *code = curr_ta->current_student->course->code;

                printf("TA: %s is serving %s from %s\n", curr_ta->name, stu_name, code);
            }
            curr_ta = curr_ta->next;
        }
    }
}

/*  list all students in queue (for testing and debugging)
 *   maybe suggest it is useful for debugging but not included in marking? 
 */ 
void print_full_queue(Student *stu_list) {
    Student *curr_student = stu_list;
    int i = 0;
    while (curr_student != NULL) {
        i += 1;

        printf("%d:%s \n", i, curr_student->name);
        printf("\t%ld: arrival time\n", *(curr_student->arrival_time));

        if (curr_student->next_overall == NULL) {
            printf("\tNULL: next overall\n");
        } else {
            printf("\t%s: next overall\n", curr_student->next_overall->name);
        }

        if (curr_student->next_course == NULL) {
            printf("\tNULL: next course\n");
        } else {
            printf("\t%s: next course\n", curr_student->next_course->name);
        }

        printf("\n");

        curr_student = curr_student->next_overall;
    }
}

/* Prints statistics to stdout for course with this course_code
 * See example output from assignment handout for formatting.
 *
 */
int stats_by_course(Student *stu_list, char *course_code, Course *courses, int num_courses, Ta *ta_list) {

    // TODO: students will complete these next pieces but not all of this 
    //       function since we want to provide the formatting

    Course *found = find_course(courses, num_courses, course_code);
    if (found == NULL) {
        return 1;
    }

    // Count students waiting
    int students_waiting = 0;
    Student *curr_student = found->head;
    while (curr_student != NULL) {
        students_waiting += 1;
        curr_student = curr_student->next_course;
    }

    // Count students being helped
    int students_being_helped = 0;
    Ta *curr_ta = ta_list;
    while (curr_ta != NULL) {
        if (curr_ta->current_student != NULL && strcmp(curr_ta->current_student->course->code, course_code) == 0) {
            students_being_helped += 1;
        }
        curr_ta = curr_ta->next;
    }
    
    // You MUST not change the following statements or your code 
    //  will fail the testing. 

    printf("%s:%s \n", found->code, found->description);
    printf("\t%d: waiting\n", students_waiting);
    printf("\t%d: being helped currently\n", students_being_helped);
    printf("\t%d: already helped\n", found->helped);
    printf("\t%d: gave_up\n", found->bailed);
    printf("\t%f: total time waiting\n", found->wait_time);
    printf("\t%f: total time helping\n", found->help_time);

    return 0;
}

/* Dynamically allocate space for the array course list and populate it
 * according to information in the configuration file config_filename
 * Return the number of courses in the array.
 * If the configuration file can not be opened, call perror() and exit.
 */
int config_course_list(Course **courselist_ptr, char *config_filename) {
    FILE *courses_file;
    courses_file = fopen(config_filename, "r");
    if (courses_file == NULL) {
        perror("unable to open file");
        exit(1);
    }

    // Process the first line
    char line[INPUT_BUFFER_SIZE];
    fgets(line, INPUT_BUFFER_SIZE, courses_file);
    int num_courses;
    sscanf(line, "%d", &num_courses);

    // Allocate memory for the course pointer list
    if ( (*courselist_ptr = malloc(num_courses * sizeof(Course) ) ) == NULL) {
        perror("malloc failed to allocate course list");
        exit(1);
    }
    Course *course_array = *courselist_ptr;

    // Process the course lines
    int i = 0;
    while ( fgets(line, INPUT_BUFFER_SIZE, courses_file) != NULL ) {
        // Construct the course
        // Allocate and set string properties
        course_array[i].description = malloc(sizeof(char) * INPUT_BUFFER_SIZE);
        sscanf(line, "%s %[^\n]s", course_array[i].code, course_array[i].description);

        // Set other properties to default values
        course_array[i].head = NULL;
        course_array[i].tail = NULL;
        course_array[i].helped = 0;
        course_array[i].bailed = 0;
        course_array[i].wait_time = 0.0;
        course_array[i].help_time = 0.0;

        i += 1;
    }

    if (fclose(courses_file) != 0) {
        perror("fclose failed");
        exit(1);
    }

    return num_courses;
}
