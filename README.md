## YTX

Hey there! I'm here to introduce you to YTX, a stand-alone software. Think of it as a simpler, clearer version of [GnuCash](https://gnucash.org). It's useful for things like keeping track of money,
managing warehouses, and staying on top of tasks. Stick around, and I'll walk you through how easy it is to use!

### Introduction

1. When the database isn't open, you won't be able to access most functions.
2. Files
   1. New, **`Ctrl + Alt+ N`**
   2. Open
      1. Drag in
      2. Double Click
      3. History Files
      4. **`Ctrl + O`**
3. File lock means putting a special file with the same name as your database file but with a `.lock` at the end. This lock file stops the database from being opened more than once at the same time,
   preventing any issues that could arise from multiple openings.
4. Configuration
   1. Mac, `/home/.config/ytx/`, **`cmd + shift + .`** Show hidden folders
   2. Win,`\usr\AppData\Roaming\ytx`

### Family Finances

Hey everyone! Today, I'll show you how to use YTX for managing your family's finances. It's like having a personal money manager for your home. Let's dive in!

1. Preferences, **`Ctrl + ,`**
   1. General - Base Unit, set to RMB **CNY**
2. Node, insert **`Ctrl + N`**, append **`Ctrl + P`**

   1. Insert five nodes: assets, liabilities, owners' equity, income, expenses
   2. Rules, **R**
      1. Dertermin how to calculate balance in transaction table. By default, a new node follows same rule as its parent node
      2. When rules are set up correctly, the total of all nodes is usually positive. We typically use the most common rule
      3. **DICD** stands for "debit increase, credit decrease"; assets, expenses, left side minus right side
      4. **DDCI** stands for "debit decrease, credit increase", owner’s equity, income, liabilities, right side minus left side
   3. Branch, **B**
      1. Branch nodes serve as groups and cannot record transactions themselves. These five are all branch nodes
   4. Unit, **U**
      1. A new node automatically adopts the unit of measurement used by its parent node
      2. If the unit of a node matches the base unit, you don't need to set a transaction ratio because the default ratio is **1**
      3. If the node's unit differs from the base unit, you must specify the ratio for each transaction

3. Next, add nodes as necessary and initialize the balance
4. Transaction, **`Ctrl + N`**

   1. Reference date; reference related node
   2. Date
      1. Time, which is not displayed by default, will be stored in the database and will be accurate to seconds
      2. Shortcut keys are enabled for the English input method; Top five are the most frequently used at work
         1. T: Now
         2. J: Yesterday
         3. K: Tomorrow
         4. H: End of last month
         5. L: First of next month
         6. W: Last week
         7. B: Next week
         8. E: Last year
         9. N: next year
   3. Ratio refers to the conversion of this node's unit relative to the base unit. For example, 1 unit of the node equals ratio times the base unit, like 1 US dollar = 7.2 times 1 RMB
   4. Original Document, **D**
      1. There's no limit on the quantity
      2. YTX only supports local use, but documents can be backed up using network disks like Dropbox and Google Drive
   5. Status, **S**
      1. We'll use it for reconciliation. First, sort by date, then reconcile
   6. Related
      1. If the related node is empty, the content of this row won't be saved when closing the transaction table
   7. Debit, Credit, Balance
      1. Display the value of this node in the current unit, where the ratio multiplied by the value equals the base unit value
      2. If you change the node's unit, remember to adjust the ratio and debit/credit amount. **Avoid changing the node's unit unless absolutely necessary**

5. Status bar
   1. The middle one displays the total value of the node in the current unit
   2. The right one displays the result of the operation between the two nodes

### Storage

Hello everyone! Today, I'll show you how **YTX** manages storage using a simple example from my home: seven storage boxes and eight storage bags

First, let's assign numbers to the storage boxes and bags. Then, I'll digitize the information

Let's create a storage box node. Unlike accounting, storage doesn't need parameter adjustments. To insert a node, use the shortcut key **`Ctrl + N`**. To append a node, use **`Ctrl + P`**

The first storage box is created, and the process repeats for the others. At last, we'll build a node to manage items taken out

Find, **`Ctrl + F`**

You can drag to change the position of the item

Delete, the item was lost or changed hands

That's all for today's introduction. Next up, I'll dive into task tracking with a focus on Shoes Market Tracking Orders, See you next time!

### Shoes Market Tracking Orders

Hello everyone! Today, I'll introduce how **YTX** manages tasks, using the example of tracking Shoes Market Orders to demonstrate the benefits of digitization. You can find videos in Chinese
[here](https://www.youtube.com/watch?v=onG_KENd5Xk)

1. Displaying customer order details
2. Displaying vendor purchasing details
3. Displaying the completed order flow chart
4. Displaying order status in transit

Insert 7 nodes: customer, vendor, bulk goods, sample, manufacturing factory, warehouse, completed. Then converts them to branches

The customer appends two nodes, A and B, converts them to branches, and subsequently appends their contact persons

The vendor appends two nodes, E and F, then converts them to branches, and appends their materials

The manufacturing factory appends two nodes, D and C, then converts them to branches and adds their processes. Additionally, C adds three processes and converts them to branches, each adding a node
with the same name for registration

The warehouse appends a node, "leather embryo," and converts it to a branch. If necessary, warehouse can also be expanded into three-dimensional coordinates: rows, columns, aisles, to specify the
location of the leather embryo. This warehouse only registers information about the leather embryos

Completed appends three nodes: billing, conversion, and product

The basic setup is ready. Let's start the simulation

1. On April 1st, Customer A, Mr. Zhao, placed a sample order for 50 square feet of item 002 and provided color cards. The delivery time is 5-7 days

   1. Append a node after sample, name it sample002, double-click to open it.

   2. **Ctrl + N** to add a record, relate it to Mr. Zhao, note the quantity and delivery time in description. You can note the delivery date in number field, and use **`Ctrl + ;`** to quickly insert
      the date of the day. Task registration completed

2. Use leather embryo No4 from vendor F

   1. Warehouse - leather embryo: add a node, "F leather embryo No4," and double-click to open it
   2. Take 40 pieces: add a record, relate it to F-No4, and set the debit to 40
   3. Make 37 pieces of goods: add a record, relate it to sample002, and set the credit to 37
   4. Return 3 pieces: add a record, relate it to F-No4, and set the credit to 3. You can note the reason for the return in description. Adjust the date according to the actual situation
   5. Synchronize number of the production instruction card to sample002 to facilitate differentiation when receiving materials
   6. Update all vendor nodes' rules to display positive values

3. Delivered to manufacturing factory C on April 2nd
   1. Drag sample002 into C, then double-click to open it. Add a record related to spraying to register the process of the order
4. Enter the first process on April 3
   1. Drag sample002 into the branch for process one, then double-click to open it. Add a record related to process one
5. Enter the second process on April 4th
   1. Drag sample002 into the branch for process two, then double-click to open it. Add a record related to process two
6. Shock on April 5th
   1. Drag sample002 into D, then double-click to open it. Add a record related to Zhensoft
7. If necessary, you can also add a record related to the porter and note the handling fee in the description to facilitate review and settlement at the end of the month
8. Finally, add a record related to conversion. Convert the number of pieces to the number of square feet, resulting in 180 square feet
9. After retrieving materials, 53 square feet were sent to Mr. Zhao. Add a record related to Mr. Zhao, and set the credit to 53. The rest was transferred to the completed product. Add a record related
   to Completed - product, and set the credit to 127.
10. Update the rule for all completed nodes to display positive values
11. Billing, select a record related to Mr. Zhao, then press **Ctrl + J** to jump. Add a record related to billing and set the credit to 53. Finally, check these three records.
12. Now that tracking is complete, drag sample002 back to the sample branch
13. Open sample002, which contains the detailed order flow. You can reuse this node for a new order. Switch to Mr. Zhao to view the order and delivery times. Open F-No4 to view the purchase record.
14. In task tracking, we employ a single entry within a double entry system. The single entry controls the flow state, while the double entry records order contents. This setup streamlines information
    collection and can be adjusted according to the actual situation, optimizing internal store management and improving work efficiency.
15. That's all for today's introduction. In the next issue, we'll delve into the warehouse section. See you next time!
