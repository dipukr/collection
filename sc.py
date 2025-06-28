import streamlit as st

import pandas as pd

import numpy as np

 

st.title("Load Bifurcation Application")

 

# Upload Excel file with 6 sheets

uploaded_file = st.file_uploader("Upload Excel file", type=["xlsx"])

if uploaded_file:

    # Read all sheets into DataFrames

    xls = pd.ExcelFile(uploaded_file)

    df_rtm         = pd.read_excel(xls, sheet_name="RTM Permission").round(2)

    df_market      = pd.read_excel(xls, sheet_name="Market").round(2)

   

    # Convert the 'Method' column in Buyer Details and round all numerical values

    df_buyer       = pd.read_excel(xls, sheet_name="Buyer Details").round(2)

    df_buyer["Method"] = df_buyer["Method"].apply(

        lambda x: f"{round(x * 100, 2)}%" if pd.notnull(x) and isinstance(x, (float, int)) else str(x).strip()

    )

   

    df_relation    = pd.read_excel(xls, sheet_name="Relationship").round(2)

    df_requisition = pd.read_excel(xls, sheet_name="Requisition").round(2)

    df_schedule    = pd.read_excel(xls, sheet_name="Schedule").round(2)

   

    # Keep a copy of the original schedule for display

    df_schedule["Original Schedule"] = df_schedule["Schedule"]

   

    # --- Market Sheet Processing ---

   market_grouped = (

        df_market

        .groupby(["Date", "Time Block", "Plant Name"])[["GDAM", "DAM", "TAM"]]

        .sum()

        .reset_index()

    )

    df_schedule = pd.merge(

        df_schedule,

        market_grouped,

        on=["Date", "Time Block", "Plant Name"],

        how="left"

    )

    df_schedule[["GDAM", "DAM", "TAM"]] = df_schedule[["GDAM", "DAM", "TAM"]].fillna(0)

    df_schedule["Market Total"] = df_schedule["GDAM"] + df_schedule["DAM"] + df_schedule["TAM"]

    df_schedule["Adjusted Schedule"] = np.where(

        df_schedule["Market Total"] > df_schedule["Schedule"],

        df_schedule["Market Total"],

        df_schedule["Schedule"]

    )

   

    # --- RTM Permission Processing ---

    df_rtm.set_index("Plant Name", inplace=True)

    def has_rtm_permission(plant):

        if plant in df_rtm.index:

            return df_rtm.loc[plant, "RTM Permission"].strip().lower() == "yes"

        return False

    df_schedule["RTM Allowed"] = df_schedule["Plant Name"].apply(has_rtm_permission)

   

    # --- Helper Function for Weighted Allocation (Non-Fixed Percentage) ---

    def parallel_allocate(participants, remaining, allocation):

        """

        participants: list of tuples (buyer, row, weight) where weight is either Contract Value or Requisition.

        allocation: dictionary holding current allocation for each buyer.

        remaining: available energy to be allocated among these participants.

        """

        unsatisfied = participants[:]  # copy of participants

        while remaining > 0 and unsatisfied:

            total_weight = sum(weight for buyer, row, weight in unsatisfied)

            if total_weight <= 0:

                break

            allocated_this_round = 0

            next_unsatisfied = []

            for buyer, row, weight in unsatisfied:

                needed = row["Requisition"] - allocation[buyer]

                proposed = remaining * (weight / total_weight)

                alloc = min(proposed, needed)

                allocation[buyer] += alloc

                allocated_this_round += alloc

            remaining -= allocated_this_round

            next_unsatisfied = [

                (buyer, row, weight)

                for buyer, row, weight in unsatisfied

                if (row["Requisition"] - allocation[buyer]) > 1e-6

            ]

            if allocated_this_round < 1e-6:

                break

            unsatisfied = next_unsatisfied

        return remaining

 

    # --- Helper Function for Fixed Percentage Allocation ---

    def parallel_allocate_fixed_percentage(participants, remaining, allocation):

        """

        participants: list of tuples (buyer, row, pct) where pct is the fixed percentage (e.g., 30 for 30%).

        allocation: dictionary holding current allocation for each buyer.

        remaining: available energy to be allocated.

        """

        unsatisfied = participants[:]  # copy

        while remaining > 0 and unsatisfied:

            allocated_this_round = 0

            next_unsatisfied = []

            for buyer, row, pct in unsatisfied:

                needed = row["Requisition"] - allocation[buyer]

                proposed = remaining * (pct / 100.0)

                alloc = min(proposed, needed)

                allocation[buyer] += alloc

                allocated_this_round += alloc

                if (row["Requisition"] - allocation[buyer]) > 1e-6:

                    next_unsatisfied.append((buyer, row, pct))

            remaining -= allocated_this_round

            if allocated_this_round < 1e-6:

                break

            unsatisfied = next_unsatisfied

        return remaining

 

    # --- Allocation Function ---

    def allocate_load(schedule_row):

        date = schedule_row["Date"]

        time_block = schedule_row["Time Block"]

        plant = schedule_row["Plant Name"]

        available_load = schedule_row["Adjusted Schedule"] - schedule_row["Market Total"]

        if available_load < 0:

            available_load = 0

 

        # Get the list of buyers for this plant.

        plant_buyers = df_relation[df_relation["Plant Name"] == plant]["Buyer Name"].unique()

        if len(plant_buyers) == 0:

            return {}

 

        buyers_today = df_buyer[

            (df_buyer["Date"] == date) &

            (df_buyer["Buyer Name"].isin(plant_buyers))

        ].copy()

        if buyers_today.empty:

            return {}

 

        req_today = df_requisition[

            (df_requisition["Date"] == date) &

            (df_requisition["Time Block"] == time_block) &

            (df_requisition["Buyer Name"].isin(plant_buyers))

        ]

        buyers_today["Requisition"] = buyers_today["Buyer Name"].map(

            req_today.set_index("Buyer Name")["Requisition"]

        )

        buyers_today["Requisition"] = buyers_today["Requisition"].fillna(0)

       

        def override_fixed_requisition(row):

            if isinstance(row["Fixed Contract"], str) and row["Fixed Contract"].strip().lower() == "yes":

                return row["Requisition"]

            fixed_tb_val = row["Fixed TB"]

            if pd.notnull(fixed_tb_val):

                try:

                    fixed_tb_val = int(fixed_tb_val)

                    if fixed_tb_val <= time_block < fixed_tb_val + 12:

                        buyer = row["Buyer Name"]

                        fixed_req_df = df_requisition[

                            (df_requisition["Date"] == date) &

                            (df_requisition["Buyer Name"] == buyer) &

                            (df_requisition["Time Block"] >= fixed_tb_val) &

                            (df_requisition["Time Block"] < fixed_tb_val + 12)

                        ]

                        if not fixed_req_df.empty:

                            avg_req = fixed_req_df["Requisition"].mean()

                            return avg_req

                        else:

                            return row["Requisition"]

                    else:

                        return row["Requisition"]

                except:

                    return row["Requisition"]

            else:

                return row["Requisition"]

       

        buyers_today["Requisition"] = buyers_today.apply(override_fixed_requisition, axis=1)

        buyers_today["Priority"] = pd.to_numeric(buyers_today["Priority"], errors="coerce")

       

        def is_fixed_tb_active(fixed_tb):

            try:

                fixed_tb = int(fixed_tb)

                return fixed_tb <= time_block < fixed_tb + 12

            except:

                return False

        buyers_today["FixedTBActive"] = buyers_today["Fixed TB"].apply(is_fixed_tb_active)

       

        # Bump for Fixed Contract buyers.

        fixed_contract_buyers = buyers_today[

            buyers_today["Fixed Contract"].str.strip().str.lower() == "yes"

        ]

        sum_fixed = fixed_contract_buyers["Requisition"].sum()

        if available_load < sum_fixed:

            bump = sum_fixed - available_load

            available_load += bump

            schedule_row["Adjusted Schedule"] += bump

            df_schedule.loc[schedule_row.name, "Adjusted Schedule"] = schedule_row["Adjusted Schedule"]

       

        remaining = available_load

        buyers_today = buyers_today.sort_values("Priority")

        allocation = {buyer: 0 for buyer in plant_buyers}

       

        # Process each priority group sequentially.

        for prio, group in buyers_today.groupby("Priority"):

            # Process Fixed Contract buyers (they get their full requisition).

            fixed_contract = group[group["Fixed Contract"].str.strip().str.lower() == "yes"]

            for _, row in fixed_contract.iterrows():

                buyer = row["Buyer Name"]

                req = row["Requisition"]

                allocation[buyer] = req

                remaining -= req

                remaining = max(remaining, 0)

           

            # For non-fixed buyers:

            non_fixed = group[~group["Buyer Name"].isin(fixed_contract["Buyer Name"])]

            fixed_tb_buyers = non_fixed[non_fixed["FixedTBActive"] == True]

            non_fixed_tb = non_fixed[non_fixed["FixedTBActive"] == False]

           

            # Bucket-fill for fixed TB buyers.

            for _, row in fixed_tb_buyers.iterrows():

                buyer = row["Buyer Name"]

                req = row["Requisition"]

                # If remaining load is insufficient, bump the schedule to cover the full fixed TB requisition.

                if remaining < req:

                    bump = req - remaining

                    remaining += bump

                    schedule_row["Adjusted Schedule"] += bump

                    df_schedule.loc[schedule_row.name, "Adjusted Schedule"] = schedule_row["Adjusted Schedule"]

               allocation[buyer] = req

                remaining -= req

                remaining = max(remaining, 0)

           

            # Now handle the rest of the non-fixed TB buyers.

            # If there's only one buyer in the group, use bucket filling.

            if len(non_fixed_tb) == 1:

                buyer = non_fixed_tb.iloc[0]["Buyer Name"]

                row = non_fixed_tb.iloc[0]

                needed = row["Requisition"] - allocation[buyer]

                alloc = min(needed, remaining)

                allocation[buyer] += alloc

                remaining -= alloc

                remaining = max(remaining, 0)

            elif len(non_fixed_tb) > 1:

                # Group these buyers by Methodology.

                group_by_methodology = {}

                for _, row in non_fixed_tb.iterrows():

                    methodol = str(row.get("Methodology", "")).strip().lower()

                    group_by_methodology.setdefault(methodol, []).append((row["Buyer Name"], row))

                

                # Process each group according to its Methodology.

                for methodol, buyer_list in group_by_methodology.items():

                    if methodol == "fixed percentage":

                        # Use fixed percentage allocation. Get percentage from the Method column.

                        participants = []

                        for buyer, row in buyer_list:

                            try:

                                pct = float(str(row["Method"]).replace("%", "").strip())

                            except:

                                pct = 0

                            participants.append((buyer, row, pct))

                        remaining = parallel_allocate_fixed_percentage(participants, remaining, allocation)

                    elif methodol in ["contract value", "requisition"]:

                        participants = []

                        for buyer, row in buyer_list:

                            weight = row.get("Contract Value", 0) if methodol == "contract value" else row["Requisition"]

                            participants.append((buyer, row, weight))

                        remaining = parallel_allocate(participants, remaining, allocation)

                    else:

                        # Fallback to bucket filling.

                        for buyer, row in buyer_list:

                            needed = row["Requisition"] - allocation[buyer]

                            alloc = min(needed, remaining)

                            allocation[buyer] += alloc

                            remaining -= alloc

                            remaining = max(remaining, 0)

            if remaining <= 0:

                break

        

        return allocation

 

    # --- Process Each Schedule Row ---

    allocation_results = []

    for idx, row in df_schedule.iterrows():

        alloc = allocate_load(row)

        allocation_results.append(alloc)

   

    # Convert allocation dictionaries to a DataFrame.

    alloc_df = pd.DataFrame(allocation_results)

   

    # Combine schedule data and buyer allocation data.

    final_df = pd.concat([df_schedule.reset_index(drop=True), alloc_df], axis=1)

   

    # --- RTM Calculation ---

    buyer_columns = list(alloc_df.columns)

    if buyer_columns:

        final_df["Total Buyer Allocation"] = final_df[buyer_columns].sum(axis=1)

        final_df["RTM"] = (

            final_df["Adjusted Schedule"]

            - final_df["Market Total"]

            - final_df["Total Buyer Allocation"]

        )

        final_df["RTM"] = final_df["RTM"].apply(lambda x: x if x > 0 else 0)

        final_df.loc[final_df["RTM Allowed"] == False, "RTM"] = np.nan

   

    # --- Final Display ---

    display_cols = [

        "Date", "Time Block", "Original Schedule",

        "GDAM", "DAM", "TAM", "Adjusted Schedule"

    ]

    display_cols.extend(buyer_columns)

    if final_df["RTM Allowed"].any():

        display_cols.append("RTM")

    st.subheader("Bifurcated Load Table")

    st.dataframe(final_df[display_cols])