#include "../include/LibraryRestructuring.h"


LibraryRestructuring::LibraryRestructuring(const UnorderedSet<BorrowRecord> &records,
                                           const UnorderedSet<Book> &bookCollection) {
    for (const Book& book : bookCollection) {
        allBooks[book.ISBN] = book;
    }

    // Initialize bookBorrowingTime and build adjacency list graph
    HashTable<std::string, UnorderedSet<std::string>> patronBookTable = HashTable<std::string, UnorderedSet<std::string>>{};
    for (const BorrowRecord& record : records) {
        bookBorrowingTime[record.bookISBN] += Date::diffDuration(record.checkoutDate, record.returnDate);

        // Associate the patron with the book they borrowed
        patronBookTable[record.patronId].insert(record.bookISBN);
    }

    // Build the graph using patron-book associations
    for (const auto& pair : patronBookTable) {
        const UnorderedSet<std::string>& books = pair->value;
        for (const std::string& book1 : books) {
            for (const std::string& book2 : books) {
                if (book1 != book2) {
                    graph[book1].insert(book2);
                }
            }
        }
    }
}


// Cluster the graph nodes and sort clusters by average borrowing time, within each cluster, the nodes must be
// internally sorted based on "sortBy" type which can be one of "title", "author", and "yearPublished"
// HINT: You need to use both RadixSort and MergeSort implementations for the implementation of clusterAndSort
std::vector<std::vector<std::string>> LibraryRestructuring::clusterAndSort(const std::string &sortBy) {
    std::vector<std::vector<std::string>> clusters;
    HashTable<std::string, bool> visited;


    // Cluster the graph nodes
    for (const auto& pair : graph) {
        if (!visited[pair->key]) {
            std::vector<std::string> cluster;
            dfs(pair->key, cluster, visited);
            clusters.push_back(cluster);
        }
    }

    // Sort clusters by average borrowing time
    RadixSort<std::vector<std::string>> rSort(clusters, [this](const std::vector<std::string>& cluster) {
        return getAverageBorrowingTime(cluster);
    });
    rSort.sort();

//    MergeSort<std::vector<std::string>> sortTime([this](const std::vector<std::string>& a, const std::vector<std::string>& b) {
//        return getAverageBorrowingTime(a) < getAverageBorrowingTime(b);
//    });


    // Sort books within each cluster
    for (auto& cluster : clusters) {
        // Sort logic based on 'sortBy' (title, author, yearPublished)
        // Example for sorting by title
        if (sortBy == "title") {
            MergeSort<std::string> titleSort([this](const std::string& a, const std::string& b) {
                return allBooks[a].title < allBooks[b].title;
            });
            titleSort.sort(cluster);
        }
        if (sortBy == "author") {
            MergeSort<std::string> authorSort([this](const std::string& a, const std::string& b) {
                return allBooks[a].author < allBooks[b].author;
            });
            authorSort.sort(cluster);
        }
        if (sortBy == "yearPublished") {
            MergeSort<std::string> yearPublishedSort([this](const std::string& a, const std::string& b) {
                return allBooks[a].yearPublished < allBooks[b].yearPublished;
            });
            yearPublishedSort.sort(cluster);
        }
    }

    return clusters;
}


// perform a DFS search to find all the nodes connected to the pointed current ISBN
void LibraryRestructuring::dfs(const std::string &current, std::vector<std::string> &cluster,
                               HashTable<std::string, bool> &visited) {
    visited[current] = true;
    cluster.push_back(current);

    for (const auto& neighbor : graph[current]) {
        if (!visited[neighbor]) {
            dfs(neighbor, cluster, visited);
        }
    }
}


// calculate the average number of days that the books in this cluster has been borrowed
double LibraryRestructuring::getAverageBorrowingTime(const std::vector<std::string> &cluster) {
    if (cluster.empty()) return 0.0;

    double totalBorrowingTime = 0.0;
    for (const std::string& isbn : cluster) {
        totalBorrowingTime += bookBorrowingTime[isbn];
    }

    return totalBorrowingTime / cluster.size();
}
