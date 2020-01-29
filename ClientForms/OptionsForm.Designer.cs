namespace CollarControl
{
	partial class OptionsForm
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
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.hostnameBox = new System.Windows.Forms.TextBox();
			this.CheckConnectionButton = new System.Windows.Forms.Button();
			this.CancelButton = new System.Windows.Forms.Button();
			this.OkButton = new System.Windows.Forms.Button();
			this.portBox = new System.Windows.Forms.NumericUpDown();
			((System.ComponentModel.ISupportInitialize)(this.portBox)).BeginInit();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(10, 31);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(55, 13);
			this.label1.TabIndex = 0;
			this.label1.Text = "Hostname";
			this.label1.MouseDown += new System.Windows.Forms.MouseEventHandler(this.OptionsForm_MouseDown);
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(10, 57);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(26, 13);
			this.label2.TabIndex = 1;
			this.label2.Text = "Port";
			this.label2.MouseDown += new System.Windows.Forms.MouseEventHandler(this.OptionsForm_MouseDown);
			// 
			// hostnameBox
			// 
			this.hostnameBox.Location = new System.Drawing.Point(71, 28);
			this.hostnameBox.Name = "hostnameBox";
			this.hostnameBox.Size = new System.Drawing.Size(100, 20);
			this.hostnameBox.TabIndex = 2;
			this.hostnameBox.TextChanged += new System.EventHandler(this.hostnameBox_TextChanged);
			// 
			// CheckConnectionButton
			// 
			this.CheckConnectionButton.Location = new System.Drawing.Point(10, 80);
			this.CheckConnectionButton.Name = "CheckConnectionButton";
			this.CheckConnectionButton.Size = new System.Drawing.Size(160, 22);
			this.CheckConnectionButton.TabIndex = 4;
			this.CheckConnectionButton.Text = "Check Connection";
			this.CheckConnectionButton.UseVisualStyleBackColor = true;
			this.CheckConnectionButton.Click += new System.EventHandler(this.CheckConnectionButton_Click);
			// 
			// CancelButton
			// 
			this.CancelButton.Location = new System.Drawing.Point(10, 108);
			this.CancelButton.Name = "CancelButton";
			this.CancelButton.Size = new System.Drawing.Size(71, 24);
			this.CancelButton.TabIndex = 5;
			this.CancelButton.Text = "Cancel";
			this.CancelButton.UseVisualStyleBackColor = true;
			this.CancelButton.Click += new System.EventHandler(this.CancelButton_Click);
			// 
			// OkButton
			// 
			this.OkButton.Location = new System.Drawing.Point(87, 108);
			this.OkButton.Name = "OkButton";
			this.OkButton.Size = new System.Drawing.Size(83, 24);
			this.OkButton.TabIndex = 6;
			this.OkButton.Text = "Ok";
			this.OkButton.UseVisualStyleBackColor = true;
			this.OkButton.Click += new System.EventHandler(this.OkButton_Click);
			// 
			// portBox
			// 
			this.portBox.Location = new System.Drawing.Point(71, 54);
			this.portBox.Maximum = new decimal(new int[] {
            65534,
            0,
            0,
            0});
			this.portBox.Name = "portBox";
			this.portBox.Size = new System.Drawing.Size(100, 20);
			this.portBox.TabIndex = 7;
			this.portBox.ValueChanged += new System.EventHandler(this.portBox_ValueChanged);
			// 
			// OptionsForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.ClientSize = new System.Drawing.Size(182, 144);
			this.Controls.Add(this.portBox);
			this.Controls.Add(this.OkButton);
			this.Controls.Add(this.CancelButton);
			this.Controls.Add(this.CheckConnectionButton);
			this.Controls.Add(this.hostnameBox);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.label1);
			this.DoubleBuffered = true;
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
			this.Name = "OptionsForm";
			this.Text = "Options";
			this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.OptionsForm_MouseDown);
			((System.ComponentModel.ISupportInitialize)(this.portBox)).EndInit();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox hostnameBox;
		private System.Windows.Forms.Button CheckConnectionButton;
		private System.Windows.Forms.Button CancelButton;
		private System.Windows.Forms.Button OkButton;
		private System.Windows.Forms.NumericUpDown portBox;
	}
}