from mpi4py import MPI
import numpy as np
import sys
import os
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.backends.backend_pdf
import seaborn as sns
import itertools
import collections
import time

from PyPDF2 import PdfFileReader, PdfFileWriter, PdfFileMerger

import tweepy as tw
import nltk
import nltk.corpus
import stopwords
import re
import networkx
from textblob import TextBlob

import warnings
warnings.filterwarnings("ignore")


# configure MPI variables
comm = MPI.COMM_WORLD
size = comm.Get_size()
rank = comm.Get_rank()

# start timer
if rank == 0:
    start_time = time.time()

sns.set(font_scale=1.5)
sns.set_style("whitegrid")

auth = tw.OAuthHandler("C7afQ2oc5K7bQ5TfWIX0zwspY", "fRpRxoLPCkoybYxJEdh02en7YzP3KBptzjY4R8yOJxFM0GnneO")
auth.set_access_token("1176586578170449920-4w5yZTNBctU4XYnP1N9YyECbBhc8L9", "CYcVn5jNoGJbZm9pQjlyt4Hwu7Pt2tIKfVe6LdGYxTD3g")
api = tw.API(auth, wait_on_rate_limit=True)

trends_send = []
numTweets = 100

output = PdfFileWriter()

def remove_url(txt):
    return " ".join(re.sub("([^0-9A-Za-z \t])|(\w+:\/\/\S+)", "", txt).split())

if rank == 0:
    #get trends in Canada
    trends_result = api.trends_place(23424775)
    iter = 0

    for trend in trends_result[0]["trends"]:
        value = trend["name"].strip("#")
        trends_send.append(value)
        iter+=1

        if iter == size:
            break

    trends_send = np.array(trends_send, dtype="S10000")
    # trends_send = trends_send.astype(str)

trends_send = comm.bcast(trends_send, root=0)
trends_send = trends_send.astype(str)

search_word = trends_send[rank]
print("rank " + str(rank) + " processing trend: " + str(search_word) + "\n")
search_term = search_word + "-filter:retweets"

# create custom search, retrieve 1000 twets
tweets = tw.Cursor(api.search, q=search_term, lang="en", since='2019-11-01').items(100)

# remove urls
tweets_no_urls = [remove_url(tweet.text) for tweet in tweets]

# create textblob objects of the tweets
sentiment_objects = [TextBlob(tweet) for tweet in tweets_no_urls]

sum = 0

for tweet in sentiment_objects:
    sum = sum + tweet.sentiment.polarity

avgPolarity = sum/numTweets

print("\n\naverage polarity for trend " + str(search_word) + " = " + str(avgPolarity) + "\n\n")

# create list of polarity values and tweet text
sentiment_values = [[tweet.sentiment.polarity, str(tweet)] for tweet in sentiment_objects]

# create list of subjectivity values and tweet text
sentiment_sub = [[tweet.sentiment.subjectivity, str(tweet)] for tweet in sentiment_objects]

# create dataframe containing polarity value and tweet text
sentiment_df = pd.DataFrame(sentiment_values, columns=["polarity", "tweet"])
#print first 5 rows

# create dataframe containing subjectivity value and tweet text
sentiment_dfsub = pd.DataFrame(sentiment_sub, columns=["subjectivity", "tweet"])

#plot values on histogram, remove polarity values of zero
sentiment_df = sentiment_df[sentiment_df.polarity != 0]
fig, ax = plt.subplots(figsize=(8,6))
sentiment_df.hist(bins=[-1, -0.75, -0.5, -0.25, 0.0, 0.25, 0.5, 0.75, 1], ax=ax, color="purple")
plt.title("Sentiments from Tweets on " + str(search_word) + "\n")
#plt.show()

# save histogram to pdf file
fig.savefig('%s.pdf' % search_word, bbox_inches='tight')

polarities = comm.gather(avgPolarity, root=0)


# trend word and avg polarity in np array to send to master
#data = np.array([search_word, avgPolarity], dtype=object)

if rank == 0:
    polarities_sort = []
    for i in range(0, size):
        polarities_sort.append(tuple((polarities[i], i)))

    polarities_sort.sort(reverse = True)

    pdfs = []
    for trend in trends_send:
        pdfs.append('%s.pdf' % trend)
    merger = PdfFileMerger()

    for pdf in pdfs:
        merger.append(pdf)

    merger.write("output.pdf")
    merger.close()

    i = 1
    print("\n\nCanadian Twitter users are happiest to least happiest about: ")
    for x,y in polarities_sort:
        print(" " + str(i) +". " + trends_send[y])
        i = i + 1

    print("\n")


    print("--- %s seconds for program execution ---" % (time.time() - start_time))
