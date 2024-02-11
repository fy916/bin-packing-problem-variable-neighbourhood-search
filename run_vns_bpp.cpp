// Author: Feiyang Wang fy916
// Implementation for Solving the Bin Packing Problem using Variable Neighbourhood Search, Best Fit, and Minimum Bin Slack algorithms
// The program takes a list of items and the bin capacity, and expected best solution of bins as input, and returns the solution of the problem.
// For the detail implementation, please see the report.pdf

#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <fstream>
#include <cstring>


using namespace std;

/*
 * Define the CONSTANTS used
 */
long MAX_TIME;
long SHAKING_STRENGTH = 4;
long SHAKING_MAX_TRY = 2000;

/*
 * the Item class represents a simple item in the BPP problem
 */
class Item{
private:
    long item_ID;
    long item_size;


public:
    Item(long itemID, long itemSize){ //Initialize value of the item
        item_ID = itemID;
        item_size = itemSize;
    }
    ~ Item(){}
    long get_item_size(){return item_size;} //getters for encapsulating attributes
    long get_item_ID(){return item_ID;}
};


/*
 * the Item class represents a Bin in the BPP problem
 */
class Bin{
private:
    long bin_total_size;
    long bin_remainnig_size;

public:
    vector<Item> items_in_bin; //the Item vector includes items in the bin


    Bin(long binsize){ //initialize the Bin
        bin_total_size = binsize;
        bin_remainnig_size = binsize;
    }

    bool is_full(){ //check if the bin is full
        return bin_remainnig_size == 0;
    };

    void reset_bin(){ //reset the bin to empty
        bin_remainnig_size = bin_total_size;
        while(items_in_bin.size()>0){items_in_bin.erase(items_in_bin.begin());}
    }

    bool add_item_to_bin(Item item){ //add an item to the bin, and make the smallest on the top
        long item_size = item.get_item_size();

        //make the smallest item on the top in the bin
        if (item_size <= bin_remainnig_size){
            for (long i = 0; i < items_in_bin.size(); i++){
                if (item_size <= items_in_bin[i].get_item_size()){
                    items_in_bin.insert(items_in_bin.begin()+i, item);
                    bin_remainnig_size = bin_remainnig_size - item.get_item_size();
                    return true;
                }
            }

            items_in_bin.push_back(item);//if the new item is the largest, put it to the buttom
            bin_remainnig_size = bin_remainnig_size - item.get_item_size();
            return true;
        } else {
            return false;
        }
    }

    long get_remaining_size(){
        return bin_remainnig_size;
    }

    bool remove_item_from_bin(long item_id){ //remove a item from the bin
        for (long i =0; i<items_in_bin.size(); i++){
            if (items_in_bin.at(i).get_item_ID() == item_id){ //if the item is in the bin
                bin_remainnig_size += items_in_bin.at(i).get_item_size(); //update bin size
                items_in_bin.erase(items_in_bin.begin()+i);
                return true;
            }
        }
        cout<<"error deleting object"<<endl;
        return false;
    }

    bool remove_nth_item_from_bin (long nth_index){ //remove the nth element in the bin
        if (nth_index < items_in_bin.size()) {
            bin_remainnig_size += items_in_bin.at(nth_index).get_item_size(); //update remaining size
            items_in_bin.erase(items_in_bin.begin()+nth_index); // remove the nth element
            return true;
        }
        return false;
    }

    bool is_item_exists(long item_id) { //check if an item exists in the bin
        for (long i =0; i<items_in_bin.size(); i++){
            if (items_in_bin.at(i).get_item_ID() == item_id){
                return true;
            }
        }
        return false; //if not found, return false
    }

    long get_item_size(long item_id){ //check the size of the item in the bin
        for (long i =0; i<items_in_bin.size(); i++){
            if (items_in_bin.at(i).get_item_ID() == item_id){
                return items_in_bin.at(i).get_item_size(); //return the item size
            }
        }
        return 0; //if not found the item in the bin, return 0
    }

    Item get_item(long item_id){ //getter which returns the item in the bin
        for (long i =0; i<items_in_bin.size(); i++){
            if (items_in_bin.at(i).get_item_ID() == item_id){
                return items_in_bin.at(i);
            }
        }
        return Item(0,0); //if not found, return a empty item
    }

    int get_item_nums(){return items_in_bin.size();}

    bool is_empty(){ //check if the bin is empty
        if(items_in_bin.size() == 0) return true;
        return false;
    }
};


//generate random number between min and max, the same implementation in Lab codes
long rand_int(long min, long max)
{
    long div = max-min+1;
    long val =rand() % div + min;
    return val;
}




/*
 * The Solution class defines the solution of the BPP problem along with the algorithms used.
 */
class Solution{
private:
    long bin_capacity;
    long best_known_bins;
    vector<Bin> final_solution; //store the final solution
    vector<Item> original_items;
public:
    void set_bin_capacity(long capacity){ bin_capacity = capacity; }
    void set_best_known_bins(long bins){best_known_bins = bins;}
    void set_original_items(vector<Item> items){original_items = items;}
    vector<Bin> get_final_solution(){return final_solution;}


    //best fit algorithm that fits the items in the bin
    vector<Bin> best_fit(vector<Item> items){
        vector<Item> sorted_items_descending = sort_items_descending(items); // sort the items according to size first, from large to small
        vector<Bin> bins;

        for (auto item :sorted_items_descending){ //go through every item
            long best_bin_index = find_best_bin(bins, item.get_item_size()); //find the most suitable bin for the item (best fit)
            if (best_bin_index!= -1){//if there is a suitable bin
                if(!bins.at(best_bin_index).add_item_to_bin(item)){ //add the new item to the bin
                    cout<<"error adding object"<<endl;
                }
            }
            else{//if there is no suitable bin
                Bin created_bin(bin_capacity);
                if (!created_bin.add_item_to_bin(item)){ //create a new bin and put the item in it
                    cout<<"error adding object"<<endl;
                }
                bins.push_back(created_bin); //add the new bin to solutions
            }
        }
        return bins;
    }


    //finds the best bin for item to fit in, whose remaining size >= the item size and is mostly close the the item size
    long find_best_bin(vector<Bin> given_bins, long item_size){
        long best_bin_index = -1;
        for (int bin_index = 0; bin_index < given_bins.size(); bin_index++){ //go through every bin
            long current_bin_remaining_size = given_bins.at(bin_index).get_remaining_size(); //check the current bin remaining size
            if (current_bin_remaining_size >= item_size){//if the bin can include the item
                if (best_bin_index != -1){ //if there is a best bin, check if this one is better
                    long current_best_bin_remaining_size = given_bins.at(best_bin_index).get_remaining_size();
                    if (current_bin_remaining_size < current_best_bin_remaining_size){ //if this bin is better, set the current bin as best
                        best_bin_index = bin_index;
                    }
                }else{
                    best_bin_index = bin_index; //add the first bin as the best bin
                }
            }
        }
        return best_bin_index;
    }

    vector<Bin> best_fit_on_bin(vector<Bin> originalBins){ //this function applies best fit on bin solutions
        vector<Bin> final_bins;
        vector<Bin> to_be_processed;
        vector<Bin> processed;
        vector<Item> items_to_be_processed;

        for (auto bin: originalBins){ //go through the given bins
            if(bin.is_full()){//if the bin is full then directly add to the solution
                final_bins.push_back(bin);
            }else{//if the bin is not full, add to the list to be applied the best fit
                to_be_processed.push_back(bin);
            }
        }

        for(auto bin: to_be_processed){//go through the pending list
            for(auto item: bin.items_in_bin){
                items_to_be_processed.push_back(item); //extract all the elements from the list
            }
        }

        processed = best_fit(items_to_be_processed); //apply best fit on the items

        for (auto bin: processed){ //add the re-fit bins to the solution
            final_bins.push_back(bin);
        }
        return final_bins;
    }



    //This minimum bin slack fit is proposed by a research paper 'A new heuristic algorithm for the one dimensional bin packing problem'
    //The algorithm has been adapted a bit to quickly calculate a solution which is used for VNS base solution
    vector<Bin> best_fit_on_minimum_bin_slack(vector<Item> original_items){
        vector<Bin> solution;
        vector<Item> sorted_pending_items = sort_items_descending(original_items); //the pending items waiting to be added to the bin
        vector<Item> removed_from_bin_list; //store the items that are removed in the backtracking process

        long total_remaining_items_counter = original_items.size(); //note the total remaining items that are not added yet
        long pending_items_size = original_items.size();

        Bin current_bin(bin_capacity); //the current try of bin including items
        Bin best_bin(bin_capacity); //find the best solution for one bin

        //find solution for all items
        while(total_remaining_items_counter > 0){

            //trying to find best solution for the current bin
            while(pending_items_size > 0){
                long item_counter = 0;

                //go through each item in the waiting list
                while(item_counter < pending_items_size) {
                    Item current_pending_item = sorted_pending_items[item_counter]; //get the item to try to add to a bin
                    if (current_bin.add_item_to_bin(current_pending_item)){ //if the item can be added to the bin
                        sorted_pending_items.erase(sorted_pending_items.begin()+item_counter); //delete it from the pending list
                        pending_items_size = sorted_pending_items.size(); //update the pending list size
                    }else{
                        item_counter++; //if the item can not be added, seek for the next smaller item
                    }
                }

                //when one search is complete, check if result is better
                if (best_bin.is_empty()) { //if the best bin is not intialized, set it as the current one
                    best_bin = current_bin;
                }else if(best_bin.get_remaining_size() > current_bin.get_remaining_size()){
                    //if the best bin is not as full as the current bin, set the current bin as best
                    best_bin = current_bin;
                }
                if(best_bin.get_remaining_size() == 0){
                    //if the best bin is full, the bin is optimal and can be added to the solution list, skip the next back tracking process
                    break;
                }
                //if the best bin is not full, apply back tracking
                removed_from_bin_list.push_back(current_bin.items_in_bin[0]);  //add the top item in the bin to the removed list
                current_bin.remove_nth_item_from_bin(0);
                pending_items_size = sorted_pending_items.size(); //update the pending item list
            }


            //when there is no item could be tested to add to the bin, add the best bin, which is although not full, to the solution list
            for (auto item: current_bin.items_in_bin){
                removed_from_bin_list.push_back(item); //add all the current bin items to the removed waiting list
            }


            for(auto item: sorted_pending_items){
                removed_from_bin_list.push_back(item); //if the bin is full and the remaining search is skipped, add all the pending items to the removed waiting list
            }

            sorted_pending_items = sort_items_descending(removed_from_bin_list); //reset all the removed items to the pending list

            for (auto item: best_bin.items_in_bin){ //delete the items in the best bin from the pending list
                long item_index_in_pending_item = 0;
                while(item_index_in_pending_item < sorted_pending_items.size()){
                    if (sorted_pending_items[item_index_in_pending_item].get_item_ID() == item.get_item_ID()){
                        sorted_pending_items.erase(sorted_pending_items.begin()+item_index_in_pending_item);
                        break; //if found and deleted, skip the remaining search
                    } else{
                        item_index_in_pending_item++; //if not found in the list, keep searching
                    }
                }
            }

            total_remaining_items_counter -= best_bin.get_item_nums(); //update the total counter
            solution.push_back(best_bin);//add the best bin to the solution list
            best_bin.reset_bin(); //reset the bin for new round of adding
            current_bin.reset_bin();
            pending_items_size = sorted_pending_items.size();
            removed_from_bin_list.clear(); //keep finding the optimal bins until all of the items are in the bin
        }

        vector<Bin> newsln = best_fit_on_bin(solution); //use best fit for the non full bins
        if(evaluate_solution(solution,newsln)){//if the best fit is better, use the best fit solution
            return newsln;
        }
        return solution;
    }


    //the MAIN entrance of the VNS search
    vector<Bin> varaible_neighbourhood_search(){
        try{
            //record the start time
            clock_t time_start, time_fin;
            time_start = clock();
            double time_spent=0;


            vector<Bin> initial_solution = best_fit_on_minimum_bin_slack(original_items);
            vector<Bin> best_solution = initial_solution; //records the best solution
            vector<Bin> current_solution = initial_solution; // records the current solution
            int VNS_K = 6;  //total of 6 types of VNS
            int nb_index = 0; //index counter

            while(true) { //keep searching until the time is up or the solution is the best known bins
                //sort the bins, with the most empty at the first of the bin lists
                current_solution = sort_bin_according_to_remaining_size(current_solution);
                check_solution_correctness(current_solution, original_items);

                while(nb_index < VNS_K){//go through the neighbourhoods
                    time_fin=clock();
                    time_spent = (double)(time_fin-time_start)/CLOCKS_PER_SEC;//check the time when a neighbour is searched
                    if (time_spent >= MAX_TIME-2 or best_solution.size() <= best_known_bins) {//if time is up or optimal is found
                        cout<<"Time Spent: "<<time_spent <<", ";
                        if (check_solution_correctness(best_solution, original_items)){ //check integrity of the best solution
                            final_solution = best_solution; //return the best solution
                            return final_solution;
                            //if the integrity of the solution is incorrect, step back for MBS or Best fit
                            //Although the integrity test has been carried out many times and no issues were found
                            //This is a backup back tracking which is not likely to be used
                        }else if (check_solution_correctness(initial_solution, original_items)){
                            cout<<"solution incorrect"<<endl;
                            final_solution = initial_solution;
                            return final_solution;
                        }else{
                            cout<<"solution incorrect"<<endl;
                            final_solution = best_fit(original_items);
                            return final_solution;
                        }
                    }

                    bool better_solution = false;
                    //run first descent variable neighbourhood search
                    current_solution = first_descent_vns(&better_solution, nb_index, current_solution, time_start);
                    //check the correctness of the solution
                    bool if_correct = check_solution_correctness(current_solution, original_items);
                    if(!if_correct){ //if the solution is incorrect, back track to use initial solution
                        //all tests carried have not show evidence that this could go incorrect
                        //just a backup back tracking the same as the above
                        current_solution = initial_solution;
                    }

                    if (better_solution){//if the solution is better
                        best_solution = current_solution; // if the solution is better than best, set the best to the current one
                        nb_index = 0; //back to the first neighborhood to search again
                    }
                    else{
                        nb_index++; //if solution is not better, seek for the neighbourhood's solution
                    }
                }
                //since all neighbourhoods have been searched and no better solution shows, do VNS shaking
                current_solution = vns_shaking(best_solution, original_items.size(),time_start);
                nb_index = 0;
            }
        }catch (exception e){ //catch exceptions, just as a back up when runtime error occurs
            //the tests carried did not show issues and thus this piecce of codes are not likely to be executed.
            vector<Bin> initial_solution = best_fit_on_minimum_bin_slack(original_items); //if run time issues occurred, use MBS
            if (check_solution_correctness(initial_solution, original_items)){
                final_solution = initial_solution;
                return final_solution;
            } else{
                final_solution = best_fit(original_items); //if MBS incorrect, use Best fit
                return final_solution;
            }
        }
        final_solution = best_fit(original_items); //if no results found, return best fit
        return final_solution;
    }



    //the neighbourhood searches are carried in a first descent form since the complete best search may cost too much time
    vector<Bin> first_descent_vns (bool* is_better, int nb_indx, vector<Bin> given_solution, clock_t time_start){
        switch(nb_indx){
            case 0: // 1-1-1 swap
                return first_descent_vns_0(is_better, given_solution, time_start);
            case 1: // 1 to 0 swap
                return first_descent_vns_1(is_better, given_solution, time_start);
            case 2: // 1 to 1 swap
                return first_descent_vns_2(is_better, given_solution, time_start);
            case 3: // 1 to 2 swap
                return first_descent_vns_3(is_better, given_solution, time_start);
            case 4: // 2 to 2 swap
                return first_descent_vns_4(is_better, given_solution, time_start);
            case 5: // 1 to n swap
                return first_descent_vns_5(is_better, given_solution, time_start);
            default:
                return given_solution;
        }
        return given_solution;
    }

    //VNS shaking shakes at a certain strength when no better solution is found
    vector<Bin> vns_shaking(vector<Bin> given_solution, long item_nums, clock_t time_start){
        int shake_time = 0;
        int trycounter = 0;
        vector<Bin> current_solution = given_solution;
        vector<long> moved_list; //note the items that are moved already and prevent duplicate move

        //note the time
        clock_t time_fin, time_start_session;
        double time_spent=0;
        double time_spent_session= 0;
        time_start_session = clock();

        //set the shake times, and the total allowed operating trys to avoid costing too much time
        while(shake_time < SHAKING_STRENGTH && trycounter<SHAKING_MAX_TRY){
            time_fin=clock();
            time_spent = (double)(time_fin-time_start)/CLOCKS_PER_SEC;
            time_spent_session = (double)(time_fin-time_start_session)/CLOCKS_PER_SEC;
            if (time_spent >= MAX_TIME -1 or time_spent_session > 5){//if time limit reaches, break the shaking process
                break;
            }
            //randomly choose two items and swap
            long index1 = rand_int(0, item_nums-1);
            long index2 = rand_int(0, item_nums-1);
            if (index1 == index2) continue; //skip if the two are the same

            bool have_moved = false;
            long moved_list_lim = moved_list.size()-1;

            for(int i = 0; i < moved_list_lim; i+=2){ //check if the two items have been swapped before
                if((moved_list[i] == index1 and moved_list[i+1] == index2) or(moved_list[i] == index2 and moved_list[i+1] == index1)){
                    have_moved = true;
                    break;
                }
            }

            if (have_moved) continue; //if has been moved, skip the current move

            bool move_successful = false;
            //add the two indexes to the move list
            vector<long> indexes_to_be_moved_A;
            vector<long> indexes_to_be_moved_B;
            indexes_to_be_moved_A.push_back(index1);
            indexes_to_be_moved_B.push_back(index2);
            moved_list.push_back(index1);
            moved_list.push_back(index2);
            //apply move between two bins
            current_solution = apply_move(&move_successful, current_solution, indexes_to_be_moved_A, indexes_to_be_moved_B);
            if (move_successful) shake_time++; //if move successful, add the shake successful counter
            trycounter++;
        }
//        cout<<"VNS shaking! for "<< shake_time << " times! "<<endl;
        return current_solution;
    }



    //Extract three items individually from bin ABC, and insert them back to BC if possible
    vector<Bin> first_descent_vns_0(bool *is_better, vector<Bin> given_solution, clock_t time_start){
        //1-1-1 swap

        //note the time
        clock_t time_fin, time_start_session;
        double time_spent=0;
        double time_spent_session= 0;
        time_start_session = clock();
        //sort the bin to have the most empty one in the front to easierly carry out the swap
        vector<Bin> sorted_bins = sort_bin_according_to_remaining_size(given_solution);

        //go through the three bins
        for(int i = 0; i < sorted_bins.size(); i++){
            for (int j = i+1; j < sorted_bins.size(); j++){
                for (int k = j+1; k< sorted_bins.size(); k++){
                    time_fin=clock();
                    time_spent = (double)(time_fin-time_start)/CLOCKS_PER_SEC;
                    time_spent_session = (double)(time_fin-time_start_session)/CLOCKS_PER_SEC;
                    if (time_spent >= MAX_TIME -1 or time_spent_session > 5){ //check the time and return if time is up
                        return given_solution;
                    }


                    //if any of those bins is empty, the operation could not be carried out and skip
                    if (sorted_bins[i].get_remaining_size() == 0
                        or sorted_bins[j].get_remaining_size() == 0
                        or sorted_bins[k].get_remaining_size() == 0){
                        continue;
                    }

                    //get the three bin indexes to move, remaining space of i is >= j's and j's >=k's
                    vector<long> moving_indexes;
                    moving_indexes.push_back(i);
                    moving_indexes.push_back(j);
                    moving_indexes.push_back(k);
                    bool move_successful = false;
                    //try to move the elements across bins
                    vector<Bin> current_solution = apply_move_across_bins(&move_successful, sorted_bins, moving_indexes);

                    //if moved, check if the solution is better
                    if (move_successful) {
                        if (evaluate_solution(given_solution, current_solution)) {
                            //first descent, if found directly return
                            *is_better = true;
                            return current_solution;

                        }
                    }
                }
            }
        }
        return given_solution;
    }


    //choose one bin to move items from, this function will move all items from the bin as possible to other bins
    vector<Bin> first_descent_vns_1 (bool *is_better, vector<Bin> given_solution, clock_t time_start){
        //move action 1-0
        vector<Bin> best_solution = given_solution;
        vector<Bin> current_solution = best_solution;

        //note the start time
        clock_t time_fin, time_start_session;
        double time_spent=0;
        double time_spent_session= 0;
        time_start_session = clock();

        //find a bin to move items from
        for (int from_bin = 0; from_bin < given_solution.size(); from_bin++){
            time_fin=clock();
            time_spent = (double)(time_fin-time_start)/CLOCKS_PER_SEC;
            time_spent_session = (double)(time_fin-time_start_session)/CLOCKS_PER_SEC;
            if (time_spent >= MAX_TIME -1 or time_spent_session > 5){ //if the time is up, break the search
                return given_solution;
            }


            bool move_successful = false;
            //apply move from the bin
            current_solution = apply_move(&move_successful, given_solution, from_bin);
            if (move_successful) {
                if (evaluate_solution(best_solution, current_solution)) {
                    //first descent, if found directly return
                    *is_better = true;
                    return current_solution;
                }
            }

        }
        return best_solution;
    }


    //choose two bins to move items in between
    vector<Bin> first_descent_vns_2 (bool *is_better, vector<Bin> given_solution, clock_t time_start){
        //1-1 swap
        vector<Bin> best_solution = given_solution;
        vector<Bin> current_solution = best_solution;

        //note the start time
        clock_t time_fin, time_start_session;
        double time_spent=0;
        double time_spent_session= 0;
        time_start_session = clock();

        //find two bins to move items from
        for (int bin_A_index = 0; bin_A_index < given_solution.size(); bin_A_index++){
            for (int bin_B_index = bin_A_index+1; bin_B_index < given_solution.size(); bin_B_index++) {
                //two bins in which elements exchange have been set, now start to swap elements
                Bin binA = given_solution.at(bin_A_index);
                Bin binB = given_solution.at(bin_B_index);
                //get one element from each bin and try to swap
                for (int item_index_in_bin_A = 0; item_index_in_bin_A < binA.get_item_nums(); item_index_in_bin_A++){
                    for (int item_index_in_bin_B = 0; item_index_in_bin_B < binB.get_item_nums(); item_index_in_bin_B++){
                        time_fin=clock();
                        time_spent = (double)(time_fin-time_start)/CLOCKS_PER_SEC;
                        time_spent_session = (double)(time_fin-time_start_session)/CLOCKS_PER_SEC;
                        if (time_spent >= MAX_TIME -1 or time_spent_session > 3){//if the time is up, break the search
                            return given_solution;
                        }

                        //add the item to the moving list
                        bool move_successful = false;
                        vector<long> indexes_to_be_moved_A;
                        vector<long> indexes_to_be_moved_B;

                        long item_ID_in_bin_A = binA.items_in_bin.at(item_index_in_bin_A).get_item_ID();
                        long item_ID_in_bin_B = binB.items_in_bin.at(item_index_in_bin_B).get_item_ID();

                        indexes_to_be_moved_A.push_back(item_ID_in_bin_A);
                        indexes_to_be_moved_B.push_back(item_ID_in_bin_B);

                        //apply the move from two bins
                        current_solution = apply_move(&move_successful, given_solution, indexes_to_be_moved_A, indexes_to_be_moved_B);
                        if (move_successful){
                            if (evaluate_solution(best_solution, current_solution)){
                                //first descent, if found directly return
                                *is_better = true;
                                return current_solution;
                            }
                        }
                    }
                }
            }
        }
        return best_solution;
    }

    //choose two bins to move items in between, and one from bin A and two from bin B
    vector<Bin> first_descent_vns_3 (bool *is_better, vector<Bin> given_solution, clock_t time_start){
        //1-2 swap
        vector<Bin> best_solution = given_solution;
        vector<Bin> current_solution = best_solution;

        //note the start time
        clock_t time_fin, time_start_session;
        double time_spent=0;
        double time_spent_session= 0;
        time_start_session = clock();

        //find two bins to move items from
        for (int bin_A_index = 0; bin_A_index < given_solution.size(); bin_A_index++){
            for (int bin_B_index = 0; bin_B_index < given_solution.size(); bin_B_index++) {
                time_fin=clock();
                time_spent = (double)(time_fin-time_start)/CLOCKS_PER_SEC;
                time_spent_session = (double)(time_fin-time_start_session)/CLOCKS_PER_SEC;
                if (time_spent >= MAX_TIME -1 or time_spent_session > 3){//if the time is up, break the search
                    return given_solution;
                }
                if (bin_A_index == bin_B_index) continue;

                //two bins in which elements exchange have been set, now start to swap elements
                Bin binA = given_solution.at(bin_A_index);
                Bin binB = given_solution.at(bin_B_index);
                //get one element from each bin and try to swap

                //get one item from bin A and two from bin B
                for (int item_index_in_bin_A = 0; item_index_in_bin_A < binA.get_item_nums(); item_index_in_bin_A++){
                    for (int item_index_in_bin_B1  = 0; item_index_in_bin_B1 < binB.get_item_nums(); item_index_in_bin_B1++){
                        for (int item_index_in_bin_B2 = item_index_in_bin_B1+1; item_index_in_bin_B2 < binB.get_item_nums(); item_index_in_bin_B2++){

                            //add the item to the moving list
                            bool move_successful = false;
                            vector<long> indexes_to_be_moved_A;
                            vector<long> indexes_to_be_moved_B;

                            long item_ID_in_bin_A = binA.items_in_bin.at(item_index_in_bin_A).get_item_ID();
                            long item_ID_in_bin_B1 = binB.items_in_bin.at(item_index_in_bin_B1).get_item_ID();
                            long item_ID_in_bin_B2 = binB.items_in_bin.at(item_index_in_bin_B2).get_item_ID();
                            indexes_to_be_moved_A.push_back(item_ID_in_bin_A);
                            indexes_to_be_moved_B.push_back(item_ID_in_bin_B1);
                            indexes_to_be_moved_B.push_back(item_ID_in_bin_B2);

                            //apply the move from two bins
                            current_solution = apply_move(&move_successful, given_solution, indexes_to_be_moved_A, indexes_to_be_moved_B);
                            if (move_successful){
                                if (evaluate_solution(best_solution, current_solution)){
                                    //first descent, if found directly return
                                    *is_better = true;
                                    return current_solution;
                                }
                            }
                        }
                    }
                }
            }
        }
        return best_solution;
    }




    //choose two bins to move items in between, two items from bin A and two from bin B
    vector<Bin> first_descent_vns_4 (bool *is_better, vector<Bin> given_solution, clock_t time_start){
        //2-2 swap
        vector<Bin> best_solution = given_solution;
        vector<Bin> current_solution = best_solution;

        //note the start time
        clock_t time_fin, time_start_session;
        double time_spent=0;
        double time_spent_session= 0;
        time_start_session = clock();

        //find two bins to move items from
        for (int bin_A_index = 0; bin_A_index < given_solution.size(); bin_A_index++){
            for (int bin_B_index = 0; bin_B_index < given_solution.size(); bin_B_index++) {
                time_fin=clock();
                time_spent = (double)(time_fin-time_start)/CLOCKS_PER_SEC;
                time_spent_session = (double)(time_fin-time_start_session)/CLOCKS_PER_SEC;
                if (time_spent >= MAX_TIME -1 or time_spent_session > 3){//if the time is up, break the search
                    return given_solution;
                }

                if (bin_A_index == bin_B_index) continue;

                //two bins in which elements exchange have been set, now start to swap elements
                Bin binA = given_solution.at(bin_A_index);
                Bin binB = given_solution.at(bin_B_index);
                //get one element from each bin and try to swap

                //get one item from bin A and two from bin B
                for (int item_index_in_bin_A1 = 0; item_index_in_bin_A1 < binA.get_item_nums(); item_index_in_bin_A1++){
                    for (int item_index_in_bin_A2 = item_index_in_bin_A1 + 1; item_index_in_bin_A2 < binA.get_item_nums(); item_index_in_bin_A2++){

                        for (int item_index_in_bin_B1  = 0; item_index_in_bin_B1 < binB.get_item_nums(); item_index_in_bin_B1++){
                            for (int item_index_in_bin_B2 = item_index_in_bin_B1+1; item_index_in_bin_B2 < binB.get_item_nums(); item_index_in_bin_B2++){

                                //add the item to the moving list
                                bool move_successful = false;
                                vector<long> indexes_to_be_moved_A;
                                vector<long> indexes_to_be_moved_B;

                                long item_ID_in_bin_A1 = binA.items_in_bin.at(item_index_in_bin_A1).get_item_ID();
                                long item_ID_in_bin_A2 = binA.items_in_bin.at(item_index_in_bin_A2).get_item_ID();

                                long item_ID_in_bin_B1 = binB.items_in_bin.at(item_index_in_bin_B1).get_item_ID();
                                long item_ID_in_bin_B2 = binB.items_in_bin.at(item_index_in_bin_B2).get_item_ID();
                                indexes_to_be_moved_A.push_back(item_ID_in_bin_A1);
                                indexes_to_be_moved_A.push_back(item_ID_in_bin_A2);
                                indexes_to_be_moved_B.push_back(item_ID_in_bin_B1);
                                indexes_to_be_moved_B.push_back(item_ID_in_bin_B2);

                                //apply the move from two bins
                                current_solution = apply_move(&move_successful, given_solution, indexes_to_be_moved_A, indexes_to_be_moved_B);
                                if (move_successful){
                                    if (evaluate_solution(best_solution, current_solution)){
                                        //first descent, if found directly return
                                        *is_better = true;
                                        return current_solution;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return best_solution;
    }




    vector<Bin> first_descent_vns_5 (bool *is_better, vector<Bin> given_solution, clock_t time_start){
        //1-n swap, to find optimal solution
        vector<Bin> current_solution = given_solution;
        current_solution = sort_bin_according_to_remaining_size(current_solution);


        //note the start time
        clock_t time_fin, time_start_session;
        double time_spent=0;
        double time_spent_session= 0;
        time_start_session = clock();

        //calculate the full bin start index to reduce analysis time
        long full_bin_starts_at = 0;
        for (long from_bin_index = 0; from_bin_index < current_solution.size(); from_bin_index++){
            if (current_solution[from_bin_index].get_remaining_size() == 0) {
                full_bin_starts_at = from_bin_index;
                break;
            }
        }


        //select one bin
        for (long from_bin_index = 0 ; from_bin_index < full_bin_starts_at; from_bin_index++){
            //select the bin from which n items to be swapped
            for(long multiple_items_bin_index = current_solution.size()-1; multiple_items_bin_index >=0; multiple_items_bin_index--){
                time_fin=clock();
                time_spent = (double)(time_fin-time_start)/CLOCKS_PER_SEC;
                time_spent_session = (double)(time_fin-time_start_session)/CLOCKS_PER_SEC;
                if (time_spent >= MAX_TIME -1 or time_spent_session > 3){//if the time is up, break the search
                    return given_solution;
                }

                //if all bins are searched, no bettter result
                if (from_bin_index == multiple_items_bin_index) {
                    *is_better = false;
                    return given_solution;
                }
                bool move_successful = false;
                //move one item in bin A with items in bin B
                current_solution = apply_move(&move_successful, current_solution, from_bin_index, multiple_items_bin_index);
                if (move_successful){
                    *is_better = true;
                    return current_solution;
                }
            }
        }
        *is_better = false;
        return given_solution;
    }











    //this function sorts the bin according to the remaining size, in descending order. The first is the most available bin
    vector<Bin> sort_bin_according_to_remaining_size(vector<Bin> given_bins){
        vector<Bin> new_bin;
        for (auto eachbin: given_bins){ //go through every bin in the bin lists
            bool item_added = false;
            for (long index_newbin = 0; index_newbin < new_bin.size(); index_newbin++){ //compare the current bin with bins in the new list
                //if the current bin is more empty than the bin at index in the new list, insert the bin at index in the new list
                if (eachbin.get_remaining_size() > new_bin[index_newbin].get_remaining_size()){
                    new_bin.insert(new_bin.begin()+index_newbin, eachbin);
                    item_added = true;
                    break; //if added, skip the next search
                }
            }
            //if the bin's remaining size is not larger than any in the new list, append it to the end of new list
            if (!item_added) new_bin.push_back(eachbin);
        }
        return new_bin;
    }

    //sort the items in descending order
    vector<Item> sort_items_descending(vector<Item> original_items){
        vector<Item> sorted_items_descending;
        bool item_added = false;
        for (int item_index = 0; item_index < original_items.size(); item_index++){// go through every item in the list
            item_added = false;
            Item item_inserted =original_items[item_index];
            long item_size_at_index = item_inserted.get_item_size();

            long sorted_items_index = 0;
            long sorted_items_vector_size = sorted_items_descending.size();
            while(sorted_items_index < sorted_items_vector_size and !item_added){ //compare the current item with items in the new list
                long sorted_item_size_at_index = sorted_items_descending.at(sorted_items_index).get_item_size();
                //if the current item is larger than the item of index in the new list, insert the item at index in the new list
                if (item_size_at_index > sorted_item_size_at_index){
                    sorted_items_descending.insert(sorted_items_descending.begin()+sorted_items_index, item_inserted);
                    item_added = true;
                }
                sorted_items_index++;
            }
            //if the item is not larger than any in the new list, append it to the end of new list
            if (!item_added){
                sorted_items_descending.push_back(item_inserted);
                item_added = true;
            }
        }
        return sorted_items_descending;
    }


    //this function moves items in the case 1-1-1, which moves item from bin0 to bin1 or bin2, and swap bin1 and bin2
    vector<Bin> apply_move_across_bins(bool* move_successful,  vector<Bin> given_bin, vector<long> indexes_to_be_moved){
        vector<Bin> current_sln = given_bin;

        //move from bin0 to other two bins, because bin0's remaining size >= bin1 and bin2
        Bin bin_0 = current_sln[indexes_to_be_moved[0]];
        Bin bin_1 = current_sln[indexes_to_be_moved[1]];
        Bin bin_2 = current_sln[indexes_to_be_moved[2]];

        long bin_0_rem_size = bin_0.get_remaining_size();
        long bin_1_rem_size = bin_1.get_remaining_size();
        long bin_2_rem_size = bin_2.get_remaining_size();

        //go through every item in bin0 bin1 and bin2, to check if they can be swapped and insert the bin0 item
        //to bin1 or bin2
        for(auto item_in_b0: bin_0.items_in_bin){
            //if the item in bin0 is too large, skip the swap
            if (item_in_b0.get_item_size() > bin_1_rem_size + bin_2_rem_size) continue;
            long item_b0_size = item_in_b0.get_item_size();
            for (auto item_in_b1: bin_1.items_in_bin){
                long item_b1_size = item_in_b1.get_item_size();
                for (auto item_in_b2: bin_2.items_in_bin){
                    long item_b2_size = item_in_b2.get_item_size();

                    if (item_b1_size > item_b2_size){
                        if (item_b1_size <= bin_2_rem_size + item_b2_size) {
                            if (item_b0_size + item_b2_size <= bin_1_rem_size + item_b1_size) {
                                // 0->1 2->1 1->2 case
                                //remove three items from the bin
                                current_sln[indexes_to_be_moved[0]].remove_item_from_bin(item_in_b0.get_item_ID());
                                current_sln[indexes_to_be_moved[1]].remove_item_from_bin(item_in_b1.get_item_ID());
                                current_sln[indexes_to_be_moved[2]].remove_item_from_bin(item_in_b2.get_item_ID());

                                //insert three items into the bin, item0 to bin1, item2 to bin1, item1 to bin2
                                current_sln[indexes_to_be_moved[1]].add_item_to_bin(item_in_b0);
                                current_sln[indexes_to_be_moved[1]].add_item_to_bin(item_in_b2);
                                current_sln[indexes_to_be_moved[2]].add_item_to_bin(item_in_b1);

                                //if bin0 is now empty, remove it from the solution list
                                if(current_sln.at(indexes_to_be_moved[0]).is_empty()){
                                    current_sln.erase(current_sln.begin()+indexes_to_be_moved[0]);
                                }
                                *move_successful = true;
                                return current_sln;
                            }
                        }
                    }else{
                        if (item_b2_size <= bin_1_rem_size + item_b1_size) {
                            if (item_b0_size + item_b1_size <= bin_2_rem_size + item_b2_size){
                                // 0->2 2->1 1->2 case
                                //remove three items from the bin
                                current_sln[indexes_to_be_moved[0]].remove_item_from_bin(item_in_b0.get_item_ID());
                                current_sln[indexes_to_be_moved[1]].remove_item_from_bin(item_in_b1.get_item_ID());
                                current_sln[indexes_to_be_moved[2]].remove_item_from_bin(item_in_b2.get_item_ID());

                                //insert three items into the bin, item0 to bin2, item2 to bin1, item1 to bin2
                                current_sln[indexes_to_be_moved[2]].add_item_to_bin(item_in_b0);
                                current_sln[indexes_to_be_moved[1]].add_item_to_bin(item_in_b2);
                                current_sln[indexes_to_be_moved[2]].add_item_to_bin(item_in_b1);

                                //if bin0 is now empty, remove it from the solution list
                                if(current_sln.at(indexes_to_be_moved[0]).is_empty()){
                                    current_sln.erase(current_sln.begin()+indexes_to_be_moved[0]);
                                }
                                *move_successful = true;
                                return current_sln;
                            }
                        }
                    }
                }
            }
        }
        *move_successful = false;
        return current_sln;
    }

    //this function applies move from one bin to other bins, it will try move all the items in the bin to others
    vector<Bin> apply_move(bool* move_successful,  vector<Bin> given_bin, long from_bin_index){
        vector<Bin> new_bin = given_bin;
        long given_bin_size = new_bin[from_bin_index].get_item_nums();
        long at_nth_in_bin = 0;
        bool obj_moved = false;

        //go through every item in the bin
        while(at_nth_in_bin < given_bin_size){
            long item_size = new_bin[from_bin_index].items_in_bin[at_nth_in_bin].get_item_size();
            Item item_to_be_moved = new_bin[from_bin_index].items_in_bin[at_nth_in_bin];

            //go through the bin list to find a bin to store the item
            for (int new_bin_index = 0; new_bin_index < new_bin.size(); new_bin_index++){
                if (new_bin_index == from_bin_index) continue; //skip the same bin
                long bin_remaining_size = new_bin[new_bin_index].get_remaining_size();

                if (bin_remaining_size < item_size) continue; //skip if the bin's remaining size is not large enough

                //if can transfer the item
                if(!new_bin.at(from_bin_index).remove_nth_item_from_bin(at_nth_in_bin)){ //remove from original bin
                    cout<<"error removing object"<<endl;
                };

                if(!new_bin.at(new_bin_index).add_item_to_bin(item_to_be_moved)){ //add to the new bin
                    cout<<"error adding object"<<endl;
                }
                obj_moved = true;
                break;
            }
            if (obj_moved){//if the object is moved, reset the search
                at_nth_in_bin = 0;
                given_bin_size = new_bin[from_bin_index].get_item_nums();
                obj_moved = false;
            }
            else{
                at_nth_in_bin++; //keep searching
            }
        }


        if(new_bin.at(from_bin_index).is_empty()){ //if the bin from which items are moved is empty, delete the bin
            new_bin.erase(new_bin.begin()+from_bin_index);
        }
        *move_successful = true;
        return new_bin;
    }

    // this function can swap multiple objects between two bins
    vector<Bin> apply_move(bool* move_successful,  vector<Bin> given_bin, vector<long> indexes_to_be_moved_A, vector<long> indexes_to_be_moved_B){
        //if given data is not enough for a move, abandon the move
        if (given_bin.size() == 0 or indexes_to_be_moved_A.size() == 0 or indexes_to_be_moved_B.size() == 0) {
            *move_successful = false;
            return given_bin;
        }

        long first_index_in_A = indexes_to_be_moved_A.at(0);
        long first_index_in_B = indexes_to_be_moved_B.at(0);
        long bin_id_moved_from_A = -1;
        long bin_id_moved_from_B = -1;

        //get the index of bins
        for(long i =0; i < given_bin.size(); i++){
            if (given_bin.at(i).is_item_exists(first_index_in_A)) bin_id_moved_from_A = i;
            if (given_bin.at(i).is_item_exists(first_index_in_B)) bin_id_moved_from_B = i;
        }

        //if the index of bins cannot be found
        if (bin_id_moved_from_A == -1 or bin_id_moved_from_B == -1){
            *move_successful = false;
            return given_bin;
        }

        //if bin A/B does not contain all the items from indexes_to_be_moved_A/B, stop moving
        for (auto index_to_be_moved_A: indexes_to_be_moved_A){
            if (!given_bin.at(bin_id_moved_from_A).is_item_exists(index_to_be_moved_A)) {
                *move_successful = false;
                return given_bin;
            }
        }
        for (auto index_to_be_moved_B: indexes_to_be_moved_B){
            if (!given_bin.at(bin_id_moved_from_B).is_item_exists(index_to_be_moved_B)) {
                *move_successful = false;
                return given_bin;
            }
        }

        //calculate the remaining bin size and item size
        long bin_A_remaining_size = given_bin.at(bin_id_moved_from_A).get_remaining_size();
        long bin_B_remaining_size = given_bin.at(bin_id_moved_from_B).get_remaining_size();

        long items_A_size = 0;
        long items_B_size = 0;
        for (auto index_to_be_moved_A: indexes_to_be_moved_A){
            items_A_size+= given_bin.at(bin_id_moved_from_A).get_item_size(index_to_be_moved_A);
        }
        for (auto index_to_be_moved_B: indexes_to_be_moved_B){
            items_B_size+= given_bin.at(bin_id_moved_from_B).get_item_size(index_to_be_moved_B);
        }



        //if not enough space for move, stop moving
        if ((bin_A_remaining_size + items_A_size - items_B_size < 0) or
            (bin_B_remaining_size + items_B_size - items_A_size < 0)){
            *move_successful = false;
            return given_bin;
        }


        //if enough space for moving, start moving
        vector<Bin> new_bin = given_bin;
        vector<Item> items_A;
        vector<Item> items_B;

        //remove item from original bin
        for (auto index_to_be_moved_A: indexes_to_be_moved_A) {
            items_A.push_back(new_bin.at(bin_id_moved_from_A).get_item(index_to_be_moved_A));
            if (!new_bin.at(bin_id_moved_from_A).remove_item_from_bin(index_to_be_moved_A)){
                cout<<"error deleting object"<<endl;
            };
        }

        for (auto index_to_be_moved_B: indexes_to_be_moved_B) {
            items_B.push_back(new_bin.at(bin_id_moved_from_B).get_item(index_to_be_moved_B));
            if (!new_bin.at(bin_id_moved_from_B).remove_item_from_bin(index_to_be_moved_B)){
                cout<<"error deleting object"<<endl;
            };
        }


        //add item to new bin (swap)
        for(auto item_A: items_A){
            if(!new_bin.at(bin_id_moved_from_B).add_item_to_bin(item_A)){
                cout<<"error adding object"<<endl;
            };
        }
        for(auto item_B: items_B){
            if(!new_bin.at(bin_id_moved_from_A).add_item_to_bin(item_B)){
                cout<<"error adding object"<<endl;
            };
        }
        *move_successful = true;
        return new_bin;
    }



    vector<Bin> apply_move(bool* move_successful,  vector<Bin> given_bin, long bin1_index, long bin2_index){
        vector<Bin> current_solution = given_bin;


        long current_bin_item_nums = current_solution[bin1_index].get_item_nums();
        //select one element in the non full bin
        for(long from_nth_element_in_bin = current_bin_item_nums-1; from_nth_element_in_bin >= 0; from_nth_element_in_bin--){

            Item itemA = current_solution[bin1_index].items_in_bin[from_nth_element_in_bin];
            long itemSize = itemA.get_item_size();
            long itemID_A = itemA.get_item_ID();


            long sizeCounter = 0;
            long swapTopNElement = 0;
            vector<Item> itemB;

            //select items from the second bin, and check the size of them and the remaining size
            for (auto itemInMultiBin: current_solution[bin2_index].items_in_bin){
                if (itemInMultiBin.get_item_size()+sizeCounter+current_solution[bin2_index].get_remaining_size() < itemSize){
                    sizeCounter += itemInMultiBin.get_item_size();
                    swapTopNElement++;
                    itemB.push_back(itemInMultiBin);
                }else{
                    sizeCounter += itemInMultiBin.get_item_size();
                    swapTopNElement++;
                    itemB.push_back(itemInMultiBin);
                    break;
                }
            }

            //if the swap is not better, search the next solution
            if (sizeCounter >  itemSize  or swapTopNElement <=1){
                continue;
            }


            //if no enough space to swap, search the next solution
            if (current_solution[bin1_index].get_remaining_size() - sizeCounter + itemSize < 0 or
                current_solution[bin2_index].get_remaining_size() + sizeCounter - itemSize < 0){
                continue;
            }

            //if the swap is feasible, do the swap
            //remove items from the bins
            current_solution[bin1_index].remove_item_from_bin(itemA.get_item_ID());
            for (auto itemB: itemB){
                current_solution[bin2_index].remove_item_from_bin(itemB.get_item_ID());
            }


            //add items (swap) items to the bins
            current_solution[bin2_index].add_item_to_bin(itemA);
            for (auto itemB: itemB){
                current_solution[bin1_index].add_item_to_bin(itemB);
            }
            //return the swapped solution
            *move_successful = true;
            return current_solution;
        }
        *move_successful = false;
        return given_bin;
    }






    //check which solution is better according to the sum of sqaure of the bin's remaining size
    bool evaluate_solution(vector<Bin> old_solution, vector<Bin> new_solution){

        //if the new solution contains less bins, it is no doubt better
        if (new_solution.size() < old_solution.size()){
            return true;
        }
        if (old_solution.size() < new_solution.size()){
            return false;
        }
        long old_sln_optimity = 0;
        long new_sln_optimity = 0;

        long avg_old_sln_counter = 0;
        long avg_old_sln_empty = 0;

        //get the sum of square of the bin's remaining size
        for (auto old_sln_bin: old_solution){
            old_sln_optimity += (old_sln_bin.get_remaining_size()) * (old_sln_bin.get_remaining_size());

        }

        for (auto new_sln_bin: new_solution){
            new_sln_optimity += (new_sln_bin.get_remaining_size()) * (new_sln_bin.get_remaining_size());
        }

        //if the new solution is better than the old one at some extent, it is better
        if ((double )new_sln_optimity/(double)old_sln_optimity>1.01) {
//                cout<<"better!"<<new_sln_optimity <<"better"<<old_sln_optimity<<endl;
            return true;
        }
        return false;
    }

    //check if the solution is correct
    bool check_solution_correctness(vector<Bin> solution, vector<Item> items){
        vector<Item> slnitemlist;
        //add the items from bins to a single list
        for (auto bin: solution){
            for (auto item: bin.items_in_bin){
                slnitemlist.push_back(item);
            }
        }

        //if the number of items contained in the solution is not the same as the original one, the solution is incorrect
        if (slnitemlist.size() != items.size()){
            cout<<"Error!"<<endl;
            return false;
        }

        //if the items in the solution have duplicated ones, the solution is incorrect
        for (long index = 0; index < slnitemlist.size(); index++) {
            for (long index2 = 0; index2 < slnitemlist.size(); index2++) {
                if (slnitemlist[index].get_item_ID() == slnitemlist[index2].get_item_ID() and index!=index2){
                    cout<<"Error!"<<endl;
                    return false;
                }
            }
        }

        //calculate the match of items between the solution and the original ones.
        long standard_num_items = items.size();
        for (auto item1: slnitemlist){
            for (auto item2: items){
                if (item1.get_item_ID() == item2.get_item_ID() and item1.get_item_size() == item2.get_item_size()){
                    standard_num_items--;
                }
            }
        }

        //if the match of items are not the number of items, it is incorrect
        if (standard_num_items != 0) {
            cout<<"Error!"<<endl;
            return false;
        }

        //otherwise, the solution is correct
        //        cout<<"correct!"<<endl;
        return true;
    }
};








/*
 * the ProblemInstance class represents a problem instance of the BPP problem
 */
class ProblemInstance{
private:
    vector<Item> original_items;
    Solution current_solution;
    string instance_id;

    long bin_capacity;
    long num_of_items;
    long best_known_bins;
public:
    //initialize the property of the problem instance in the constructor
    ProblemInstance(long bincapacity, long numofitems, long bestknownbins, string instanceid, vector<Item> originalItems){
        this->bin_capacity = bincapacity;
        this->num_of_items = numofitems;
        this->best_known_bins = bestknownbins;
        this->instance_id = instanceid;
        this->original_items = originalItems;
        current_solution.set_bin_capacity(bincapacity);
        current_solution.set_best_known_bins(bestknownbins);
        current_solution.set_original_items(originalItems);
    }

    string get_instance_id (){ return instance_id; }
    long get_bin_capacity(){ return bin_capacity; }
    long get_num_of_items(){ return num_of_items; }
    long get_best_known_bins(){ return best_known_bins; }
    vector<Bin> get_final_solution(){ return current_solution.get_final_solution(); }

    //call this function to use VNS to solve problem
    void solve_problem(){
        cout << "Problem ID: " << instance_id<< endl;
        vector<Bin> sln = current_solution.varaible_neighbourhood_search();
        cout<< "My solution bins: " << sln.size()<< ", Standard Solution bins: " << best_known_bins<< ", abs_gap: " <<sln.size()-best_known_bins<<endl;
    }
};




/*
 * This BinPackProblem class contains the all the problem instances that to be solved
 */
class BinPackProblem{
private:
    vector<ProblemInstance> problem_instances;

public:
    void add_problem_instance(ProblemInstance problem_instance){ problem_instances.push_back(problem_instance); } //add instance to the problem

    void solve_problem_instance(int index){ //call this function to solve the problem for each instance
        if (index >=0 and index < problem_instances.size()){
            problem_instances.at(index).solve_problem();
        }
    }

    long get_problem_instances_numbers(){ return problem_instances.size();}
    vector<ProblemInstance> get_problem_instances(){ return problem_instances; }
};



/*
 * This FileReader deals with the IO to the files
 */
class FileReader{
private:
    string problem_file_name;
    string solution_file_name;
    ifstream problem_file_stream;
    ofstream solution_file_stream;

public:
    FileReader(string problem_f_name, string solution_f_name) { //initialize the class with file names
        problem_file_name = problem_f_name;
        solution_file_name = solution_f_name;
    }

    bool load_problem(BinPackProblem *bin_pack_problem) { //load problems to the memory
        problem_file_stream.open(problem_file_name, ios::in);
        if (!problem_file_stream.is_open()) {
            cout << "cannot open file" << endl;
            return false;
        }

        string str;
        problem_file_stream >> str;
        long num_of_problems = strtol(str.c_str(), nullptr, 10); //the number of problems

        //for each problem, load the property
        for (int problem_counter = 0; problem_counter < num_of_problems; problem_counter++) {
            problem_file_stream >> str;
            string instance_id = str;

            problem_file_stream >> str;
            long bin_capacity = strtol(str.c_str(), nullptr, 10);

            problem_file_stream >> str;
            long num_of_items = strtol(str.c_str(), nullptr, 10);

            problem_file_stream >> str;
            long best_known_bins = strtol(str.c_str(), nullptr, 10);

            //add the items
            vector<Item> items_to_add;
            for (long item_counter = 0; item_counter < num_of_items; item_counter++) {
                problem_file_stream >> str;
                long item_size = strtol(str.c_str(), nullptr, 10);
                Item item(item_counter, item_size);
                items_to_add.push_back(item);
            }

            //initialize the problem instance for it
            ProblemInstance problem_instance(bin_capacity, num_of_items, best_known_bins, instance_id, items_to_add);
            bin_pack_problem->add_problem_instance(problem_instance);
        }

        problem_file_stream.close(); //close the stream
        if(!write_num_of_instances(num_of_problems)){ //write the number of instances to the solution file
            return false;
        }
        return true;
    }



    bool write_num_of_instances(long instances_num){
        solution_file_stream.open(solution_file_name,ios::out); //create the file
        if (!solution_file_stream.is_open()) {
            cout << "cannot write file" << endl;
            return false;
        }

        //write the number of instances to the solution file
        solution_file_stream << instances_num<< endl;
        solution_file_stream.close();
        return true;
    }



    //write solution to the file
    bool write_solution(ProblemInstance current_inst ) {
        solution_file_stream.open(solution_file_name,ios::app);//use append method
        if (!solution_file_stream.is_open()) {
            cout << "cannot write file" << endl;
            return false;
        }

        //get the solution
        vector<Bin> curr_sln = current_inst.get_final_solution();

        //write the id, objectives to the file
        solution_file_stream << "instance ID = " << current_inst.get_instance_id() << endl;
        solution_file_stream << "solution bins =   "<< curr_sln.size() << endl ;
        solution_file_stream << "expected bins =   "<< current_inst.get_best_known_bins() << endl;
        solution_file_stream << "difference =   "<< curr_sln.size()-current_inst.get_best_known_bins() << endl;

        //write the solution to the file
        int bin_counter = 0;
        for (auto each_bin: curr_sln){//items in each bin in the same line
            solution_file_stream << "Bin ID: " << bin_counter << " --- Item ID: ";
            for (auto item: each_bin.items_in_bin){
                solution_file_stream <<item.get_item_ID() << " ";
            }
            solution_file_stream << endl;
            bin_counter ++;
        }
        solution_file_stream.close();
        return true;
    }
};






//if the solution could not be written to file, print to the terminal
void print_solution(ProblemInstance current_inst){
    //get the solution
    vector<Bin> curr_sln = current_inst.get_final_solution();

    //print the id, objectives to the file
    cout << current_inst.get_instance_id() << endl;
    cout << " obj=   "<< curr_sln.size() << " \t " << curr_sln.size()-current_inst.get_best_known_bins() << endl;
    //print the solution to the file
    for (auto each_bin: curr_sln){//items in each bin in the same line
        for (auto item: each_bin.items_in_bin){
            cout<< item.get_item_ID() << " ";
        }
        cout<< endl;
    }
    cout<< endl<<endl;

}





int main(int argc, const char * argv[]) {
    cout << "Welcome to this VNS solver! "<< endl<<endl;
    //define the file names, and a default for solution file
    string problem_file_name;
    string solution_file_name = "my_solutions.txt";

    //read in the parameters
    if(argc != 7)
    {
        printf("Insufficient arguments. Please use the following options:\n   -s data_file\n   -o out_file\n   -t max_time (in sec)\n");
        return 1;
    }
    else
    {
        for(int i=1; i<argc; i=i+2)
        {
            if(strcmp(argv[i],"-s")==0)
                problem_file_name = argv[i+1];
            else if(strcmp(argv[i],"-o")==0)
                solution_file_name = argv[i+1];
            else if(strcmp(argv[i],"-t")==0)
                MAX_TIME = atoi(argv[i+1]);
        }
    }





    //print the info
    cout<<"Problem file name is: " << problem_file_name << endl;
    cout <<"Solution file name is: " << solution_file_name << endl;
    cout<<"Max time allowed: "<< MAX_TIME <<endl<<endl;

    //initialize the file reader
    FileReader filereader(problem_file_name, solution_file_name);

    //create a new problem object, in which all instances will be stored
    BinPackProblem* problem = new BinPackProblem;
    filereader.load_problem(problem);

    //print the messages
    cout<<"Problems successfully loaded! " <<endl;
    cout<<"Total of "<< problem->get_problem_instances_numbers() << " problems. "<< endl << endl;
    cout<<"Start solving! " <<endl<<endl;
    cout<<"---------------------Status---------------------" <<endl;

    //note the time
    clock_t time_start, time_fin;
    time_start = clock();
    double time_spent=0;



    //solve all the problems
    for (int i = 0; i < problem->get_problem_instances_numbers(); i++){
        srand(39);
        problem->solve_problem_instance(i);
        cout  <<"Start writing solutions to file " << solution_file_name << endl;
        if (filereader.write_solution(problem->get_problem_instances()[i])){
            cout <<"Solutions successfully written to " << solution_file_name << endl<<endl;
        }else{
            cout <<"Fail to write solution to file, solution is: " << endl<<endl;
            print_solution(problem->get_problem_instances()[i]);
        }

    }
    //print the time spent
    time_fin=clock();
    time_spent = (double)(time_fin-time_start)/CLOCKS_PER_SEC;
    cout<<"------------------------- " <<endl;
    cout <<"All problems are solved! " << endl;
    cout<<"Time Spent: " << time_spent<<endl;
    cout<<"Thanks for using! " << endl;

    delete problem;


    return 0;
}

