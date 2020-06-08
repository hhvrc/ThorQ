namespace ThorQ
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.BtnSetMaster = new System.Windows.Forms.Button();
            this.ComboboxActiveUsers = new System.Windows.Forms.ComboBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.MasterInfoBox = new System.Windows.Forms.GroupBox();
            this.OutSessionID = new System.Windows.Forms.Label();
            this.OutUsername = new System.Windows.Forms.Label();
            this.groupBox1.SuspendLayout();
            this.MasterInfoBox.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.BtnSetMaster);
            this.groupBox1.Controls.Add(this.ComboboxActiveUsers);
            this.groupBox1.Location = new System.Drawing.Point(3, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(345, 77);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Active Users";
            // 
            // BtnSetMaster
            // 
            this.BtnSetMaster.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.BtnSetMaster.Location = new System.Drawing.Point(7, 47);
            this.BtnSetMaster.Name = "BtnSetMaster";
            this.BtnSetMaster.Size = new System.Drawing.Size(332, 23);
            this.BtnSetMaster.TabIndex = 2;
            this.BtnSetMaster.Text = "Set As Master UwU";
            this.BtnSetMaster.UseVisualStyleBackColor = true;
            this.BtnSetMaster.Click += new System.EventHandler(this.BtnSetMaster_Click);
            // 
            // ComboboxActiveUsers
            // 
            this.ComboboxActiveUsers.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.ComboboxActiveUsers.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.ComboboxActiveUsers.FormattingEnabled = true;
            this.ComboboxActiveUsers.Location = new System.Drawing.Point(9, 19);
            this.ComboboxActiveUsers.Name = "ComboboxActiveUsers";
            this.ComboboxActiveUsers.Size = new System.Drawing.Size(330, 21);
            this.ComboboxActiveUsers.TabIndex = 0;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 47);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(55, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "SessionID";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 22);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(55, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Username";
            // 
            // MasterInfoBox
            // 
            this.MasterInfoBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.MasterInfoBox.Controls.Add(this.OutSessionID);
            this.MasterInfoBox.Controls.Add(this.OutUsername);
            this.MasterInfoBox.Controls.Add(this.label1);
            this.MasterInfoBox.Controls.Add(this.label2);
            this.MasterInfoBox.Location = new System.Drawing.Point(354, 12);
            this.MasterInfoBox.Name = "MasterInfoBox";
            this.MasterInfoBox.Size = new System.Drawing.Size(488, 77);
            this.MasterInfoBox.TabIndex = 1;
            this.MasterInfoBox.TabStop = false;
            this.MasterInfoBox.Text = "Master Information";
            // 
            // OutSessionID
            // 
            this.OutSessionID.AutoSize = true;
            this.OutSessionID.Location = new System.Drawing.Point(67, 47);
            this.OutSessionID.Name = "OutSessionID";
            this.OutSessionID.Size = new System.Drawing.Size(0, 13);
            this.OutSessionID.TabIndex = 5;
            // 
            // OutUsername
            // 
            this.OutUsername.AutoSize = true;
            this.OutUsername.Location = new System.Drawing.Point(67, 22);
            this.OutUsername.Name = "OutUsername";
            this.OutUsername.Size = new System.Drawing.Size(0, 13);
            this.OutUsername.TabIndex = 4;
            // 
            // SubmissiveControlForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(854, 95);
            this.Controls.Add(this.MasterInfoBox);
            this.Controls.Add(this.groupBox1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(870, 134);
            this.MinimumSize = new System.Drawing.Size(870, 134);
            this.Name = "SubmissiveControlForm";
            this.Text = "Submissive Control Client";
            this.groupBox1.ResumeLayout(false);
            this.MasterInfoBox.ResumeLayout(false);
            this.MasterInfoBox.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox MasterInfoBox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button BtnSetMaster;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox ComboboxActiveUsers;
        private System.Windows.Forms.Label OutSessionID;
        private System.Windows.Forms.Label OutUsername;
    }
}

